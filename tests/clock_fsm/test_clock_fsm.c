#include "clock_fsm.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    clock_fsm_datetime_t date;
    clock_fsm_alarm_t alarm;
    uint8_t brightness, hardware;
    int fail_read, fail_save, fail_preview;
    int reads, saves;
} backend_t;

static int read_date(void *p, clock_fsm_datetime_t *out)
{ backend_t *b=p; b->reads++; *out=b->date; return b->fail_read; }
static int save_time(void *p, const clock_fsm_datetime_t *v)
{
    backend_t *b=p; if (b->fail_save) return b->fail_save;
    b->date.hour=v->hour; b->date.minute=v->minute;
    b->date.second=v->second; b->saves++; return 0;
}
static int save_date(void *p, const clock_fsm_datetime_t *v)
{
    backend_t *b=p; if (b->fail_save) return b->fail_save;
    b->date.year=v->year; b->date.month=v->month;
    b->date.day=v->day; b->saves++; return 0;
}
static int read_alarm(void *p, clock_fsm_alarm_t *out)
{ *out=((backend_t *)p)->alarm; return 0; }
static int save_alarm(void *p, const clock_fsm_alarm_t *v)
{ backend_t *b=p; if(b->fail_save)return b->fail_save; b->alarm=*v; return 0; }
static int read_light(void *p, uint8_t *out)
{ *out=((backend_t *)p)->brightness; return 0; }
static int preview(void *p, uint8_t v)
{ backend_t *b=p; if(b->fail_preview)return b->fail_preview; b->hardware=v; return 0; }
static int save_light(void *p, uint8_t v)
{ backend_t *b=p; if(b->fail_save)return b->fail_save; b->brightness=v; return 0; }
static void setup(clock_fsm_t *f, backend_t *b)
{
    *b=(backend_t){.date={24,1,31,19,34,56},.alarm={7,30,0},
                   .brightness=9,.hardware=9};
    clock_fsm_ops_t ops={b,read_date,save_time,save_date,read_alarm,
        save_alarm,read_light,preview,save_light};
    assert(clock_fsm_init(f,&ops,0)==0);
}
static int key(clock_fsm_t *f, clock_fsm_event_type_t type, int64_t t)
{ clock_fsm_event_t e={type,CLOCK_FSM_KEY,t}; return clock_fsm_dispatch(f,&e,t); }
static void go(clock_fsm_t *f, clock_fsm_event_type_t type, int64_t t)
{ assert(key(f,type,t)==0); }

int main(void)
{
    clock_fsm_t f; backend_t b; int out[9];
    setup(&f,&b);
    go(&f,CLOCK_FSM_CONFIRM,0);
    assert(f.state==CLOCK_FSM_TIME_EDIT);
    go(&f,CLOCK_FSM_UP,10);
    assert(f.draft_datetime.hour==23 && b.date.hour==19);
    int reads=b.reads;
    assert(clock_fsm_render(&f,10,out)==0 && out[1]==2 && out[2]==3);
    assert(b.reads==reads);
    assert(clock_fsm_render(&f,500010,out)==0 && out[1]==10 && out[2]==3);
    go(&f,CLOCK_FSM_RIGHT,500011);
    go(&f,CLOCK_FSM_UP,500012);
    assert(f.draft_datetime.hour==20);
    b.fail_save=77;
    assert(key(&f,CLOCK_FSM_CONFIRM,500020)==77);
    assert(f.state==CLOCK_FSM_TIME_EDIT && f.draft_datetime.hour==20);
    b.fail_save=0;
    go(&f,CLOCK_FSM_CONFIRM,500030);
    assert(f.state==CLOCK_FSM_TIME_VIEW && b.date.hour==20);
    puts("PASS: time digit edit, immediate render, blink, failed save retention");

    setup(&f,&b); b.fail_read=41;
    assert(key(&f,CLOCK_FSM_CONFIRM,0)==41 && f.state==CLOCK_FSM_TIME_VIEW);
    b.fail_read=0; b.date.month=0;
    assert(key(&f,CLOCK_FSM_CONFIRM,1)==CLOCK_FSM_INVALID_DATA);
    assert(f.state==CLOCK_FSM_TIME_VIEW);
    puts("PASS: load failure and invalid RTC rejection");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_DATE,0); go(&f,CLOCK_FSM_CONFIRM,0);
    for(int i=0;i<3;i++) go(&f,CLOCK_FSM_RIGHT,0);
    go(&f,CLOCK_FSM_UP,0);
    assert(f.draft_datetime.month==2 && f.draft_datetime.day==29);
    go(&f,CLOCK_FSM_LEFT,0); go(&f,CLOCK_FSM_LEFT,0);
    go(&f,CLOCK_FSM_UP,0);
    assert(f.draft_datetime.year==25 && f.draft_datetime.day==28);
    go(&f,CLOCK_FSM_CONFIRM,0);
    assert(b.date.day==28 && b.date.hour==19);
    puts("PASS: leap day clamp and date-only save");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_BRIGHTNESS,0); go(&f,CLOCK_FSM_CONFIRM,0);
    go(&f,CLOCK_FSM_UP,0);
    assert(f.draft_brightness==15 && b.hardware==15 && b.brightness==9);
    b.fail_preview=9;
    assert(key(&f,CLOCK_FSM_CANCEL,1)==9 && clock_fsm_is_editing(&f));
    b.fail_preview=0; go(&f,CLOCK_FSM_CANCEL,2);
    assert(b.hardware==9 && b.brightness==9 && !clock_fsm_is_editing(&f));
    go(&f,CLOCK_FSM_CONFIRM,3); go(&f,CLOCK_FSM_UP,4); go(&f,CLOCK_FSM_CONFIRM,5);
    assert(b.brightness==15);
    puts("PASS: brightness preview, restore failure, cancel and commit");

    setup(&f,&b); go(&f,CLOCK_FSM_CONFIRM,0);
    clock_fsm_event_t voice={CLOCK_FSM_GOTO_DATE,CLOCK_FSM_VOICE,600000};
    assert(clock_fsm_dispatch(&f,&voice,600000)==0 && clock_fsm_is_editing(&f));
    go(&f,CLOCK_FSM_CANCEL,1000000);
    voice.timestamp_us=900000;
    assert(clock_fsm_dispatch(&f,&voice,2000000)==0 && f.state==CLOCK_FSM_TIME_VIEW);
    voice.timestamp_us=2000001;
    assert(clock_fsm_dispatch(&f,&voice,2000001)==0 && f.state==CLOCK_FSM_DATE_VIEW);
    puts("PASS: editing voice rejection and stale voice rejection after exit");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_TIMER,0); go(&f,CLOCK_FSM_CONFIRM,0);
    go(&f,CLOCK_FSM_LEFT,1000000); /* running: cannot clear */
    go(&f,CLOCK_FSM_GOTO_DATE,2000000); go(&f,CLOCK_FSM_CONFIRM,2000000);
    go(&f,CLOCK_FSM_CANCEL,30000000); go(&f,CLOCK_FSM_GOTO_TIMER,30000000);
    uint64_t elapsed;
    assert(clock_fsm_elapsed_ms(&f,30000000,&elapsed)==0 && elapsed==30000);
    go(&f,CLOCK_FSM_CONFIRM,30000000);
    assert(clock_fsm_elapsed_ms(&f,40000000,&elapsed)==0 && elapsed==30000);
    go(&f,CLOCK_FSM_CONFIRM,40000000);
    assert(clock_fsm_elapsed_ms(&f,42000000,&elapsed)==0 && elapsed==32000);
    go(&f,CLOCK_FSM_CONFIRM,42000000); go(&f,CLOCK_FSM_LEFT,42000000);
    assert(clock_fsm_elapsed_ms(&f,42000000,&elapsed)==0 && elapsed==0);
    puts("PASS: cross-page stopwatch, pause/resume and guarded reset");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_ALARM,0); go(&f,CLOCK_FSM_CONFIRM,0);
    go(&f,CLOCK_FSM_RIGHT,0); go(&f,CLOCK_FSM_UP,0);
    assert(b.alarm.hour==7 && f.draft_alarm.hour==8);
    go(&f,CLOCK_FSM_CONFIRM,0); assert(b.alarm.hour==8);
    puts("PASS: alarm draft isolation and commit");

    setup(&f,&b);
    for(int i=0;i<5;i++)go(&f,CLOCK_FSM_DOWN,i);
    assert(f.state==CLOCK_FSM_TIME_VIEW);
    for(int i=0;i<5;i++)go(&f,CLOCK_FSM_UP,10+i);
    assert(f.state==CLOCK_FSM_TIME_VIEW);
    assert(clock_fsm_dispatch(NULL,NULL,0)==CLOCK_FSM_INVALID_ARG);
    puts("PASS: five-page cycle and invalid arguments");

    setup(&f,&b); go(&f,CLOCK_FSM_CONFIRM,0);
    go(&f,CLOCK_FSM_LEFT,0); assert(f.cursor==5);
    go(&f,CLOCK_FSM_RIGHT,0); assert(f.cursor==0);
    assert(clock_fsm_render(&f,499999,out)==0 && out[1]==1);
    assert(clock_fsm_render(&f,500000,out)==0 && out[1]==10);
    assert(clock_fsm_render(&f,1000000,out)==0 && out[1]==1);
    go(&f,CLOCK_FSM_CANCEL,1000000);
    for(int i=0;i<9;i++)out[i]=88;
    b.fail_read=81;
    assert(clock_fsm_render(&f,1000001,out)==81);
    for(int i=0;i<9;i++)assert(out[i]==88);
    assert(clock_fsm_needs_render(&f,1000002));
    puts("PASS: cursor wrap, exact blink boundaries and failed render isolation");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_TIMER,0); go(&f,CLOCK_FSM_CONFIRM,0);
    const int64_t hundred_hours=INT64_C(360000000000);
    assert(clock_fsm_render(&f,hundred_hours,out)==0);
    assert(out[1]==9 && out[2]==9 && out[3]==5 && out[4]==9);
    assert(out[5]==5 && out[6]==9 && out[7]==9 && out[8]==9);
    assert(clock_fsm_elapsed_ms(&f,hundred_hours,&elapsed)==0);
    assert(elapsed==UINT64_C(360000000) && f.stopwatch.running);
    puts("PASS: stopwatch display saturation preserves internal elapsed time");

    setup(&f,&b);
    assert(clock_fsm_note_key_activity(&f,1000000)==0);
    voice=(clock_fsm_event_t){CLOCK_FSM_GOTO_DATE,CLOCK_FSM_VOICE,1500000};
    assert(clock_fsm_dispatch(&f,&voice,1500000)==0 && f.state==CLOCK_FSM_TIME_VIEW);
    voice.timestamp_us=1500001;
    assert(clock_fsm_dispatch(&f,&voice,1500001)==0 && f.state==CLOCK_FSM_DATE_VIEW);
    voice.type=CLOCK_FSM_CONFIRM; voice.timestamp_us=2000000;
    assert(clock_fsm_dispatch(&f,&voice,2000000)==0 && f.state==CLOCK_FSM_DATE_VIEW);
    voice.timestamp_us=3000000;
    assert(clock_fsm_dispatch(&f,&voice,2000001)==CLOCK_FSM_INVALID_ARG);
    puts("PASS: held-key priority window and invalid voice event rejection");

    setup(&f,&b); go(&f,CLOCK_FSM_GOTO_BRIGHTNESS,0); go(&f,CLOCK_FSM_CONFIRM,0);
    b.fail_preview=51;
    assert(key(&f,CLOCK_FSM_UP,0)==51 && f.draft_brightness==9 && b.hardware==9);
    b.fail_preview=0;
    /* 多轮合法事件：任何光标位都不得产生非法时间、日期、亮度或字模。 */
    for(int page=0;page<4;page++) {
        setup(&f,&b);
        const clock_fsm_event_type_t pages[]={CLOCK_FSM_GOTO_TIME,
            CLOCK_FSM_GOTO_DATE,CLOCK_FSM_GOTO_ALARM,CLOCK_FSM_GOTO_BRIGHTNESS};
        go(&f,pages[page],0); go(&f,CLOCK_FSM_CONFIRM,0);
        for(int i=0;i<600;i++) {
            go(&f,(i%17==0)?CLOCK_FSM_RIGHT:
                  (i%3==0)?CLOCK_FSM_DOWN:CLOCK_FSM_UP,i);
            assert(clock_fsm_render(&f,i,out)==0);
            for(int j=0;j<9;j++)assert(out[j]>=0 && out[j]<=10);
        }
    }
    puts("PASS: preview failure isolation and repeated editing range checks");
    return 0;
}
