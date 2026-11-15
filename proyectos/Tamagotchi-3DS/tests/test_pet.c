#include "../source/pet.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint64_t noon=12*3600;
static void fresh(Pet *p){pet_new(p,noon,"TAMA");pet_hatch(p);}
int main(void){
 Pet p;char text[256];fresh(&p);assert(pet_valid(&p));assert(p.character<2);
 p.hunger=1;pet_action(&p,0,0,text,sizeof(text));assert(p.hunger==2);assert(p.weight==6);
 p.poop=3;pet_action(&p,1,0,text,sizeof(text));assert(p.poop==0);
 p.sick=2;pet_action(&p,2,0,text,sizeof(text));assert(p.sick==1);pet_action(&p,2,0,text,sizeof(text));assert(!p.sick);
 p.flags|=FLAG_REFUSE;unsigned training=p.training;pet_action(&p,4,0,text,sizeof(text));assert(p.training==training+1);assert(!(p.flags&FLAG_REFUSE));
 p.flags|=FLAG_CRY;pet_action(&p,3,0,text,sizeof(text));assert(!(p.flags&FLAG_CRY));
 p.points=0;pet_action(&p,7,10,text,sizeof(text));assert(!p.inventory[10]);p.points=300;pet_action(&p,7,10,text,sizeof(text));assert(p.inventory[10]==1&&p.points==100);
 assert(pet_code(&p,"ACBCABAC")==20);assert(p.inventory[20]==1);assert(pet_code(&p,"INVALID")==-1);
 pet_code(&p,"BCBACABA");pet_code(&p,"BACBCACB");pet_code(&p,"CBAACABC");assert(pet_code(&p,"ABBACBCC")==23);
 fresh(&p);p.flags|=FLAG_PAUSE;pet_tick(&p,noon+86400);assert(!p.age_minutes&&p.hunger==4);
 fresh(&p);pet_tick(&p,noon-100);assert(!p.age_minutes);
 fresh(&p);p.stage_minutes=59;pet_step(&p,12*60);assert(p.stage==1&&p.character==2);
 fresh(&p);p.stage=2;p.stage_minutes=2879;p.generation=2;p.training=9;pet_step(&p,12*60);assert(p.character>=31&&p.character<=33);
 fresh(&p);p.flags|=FLAG_BABY;p.partner=16;p.baby_sex=1;p.baby_minutes=2879;p.points=987;p.inventory[12]=1;pet_step(&p,12*60);assert(p.generation==2&&p.character==1&&p.points==987&&p.inventory[12]==1);
 fresh(&p);p.hunger=0;for(unsigned i=0;i<400;i++)pet_step(&p,12*60);assert(p.flags&FLAG_DEAD);
 fresh(&p);p.points=9998;pet_game_reward(&p,3,100,9);assert(p.points==9999&&p.weight>=5);
 fresh(&p);p.generation=2;p.stage=3;p.character=33;p.age_minutes=7*1440;pet_action(&p,9,16,text,sizeof(text));assert(p.flags&FLAG_BABY);
 fresh(&p);assert(pet_save(&p,"work/tama"));p.points=432;assert(pet_save(&p,"work/tama"));Pet q;assert(pet_load(&q,"work/tama")&&q.points==432);
 FILE *f=fopen("work/tama/mascota.dat","r+b");assert(f);fseek(f,24,SEEK_SET);fputc(0xff,f);fclose(f);assert(pet_load(&q,"work/tama")&&q.points==100);
 q.friend_count=999;assert(!pet_valid(&q));
 fresh(&p);p.friend_count=1;p.friends[0].character=16;p.friends[0].wrapped=1;p.inventory[10]=99;pet_action(&p,8,0,text,sizeof(text));assert(pet_valid(&p)&&p.inventory[10]==99);
 fresh(&p);p.parent1=47;p.parent2=46;p.stage_minutes=59;pet_step(&p,12*60);assert(p.character==48&&p.stage==3);
 fresh(&p);
 for(unsigned i=0;i<50000;i++){
  unsigned action=pet_random(&p,9);
  if(p.flags&FLAG_DEAD)fresh(&p);
  if(action==0)pet_action(&p,0,(int)pet_random(&p,2),text,sizeof(text));
  else if(action==1)pet_action(&p,1,0,text,sizeof(text));
  else if(action==2)pet_action(&p,2,0,text,sizeof(text));
  else if(action==3)pet_action(&p,3,0,text,sizeof(text));
  else if(action==4)pet_action(&p,4,0,text,sizeof(text));
  else if(action==5)pet_action(&p,7,2+(int)pet_random(&p,ITEM_COUNT-2),text,sizeof(text));
  else if(action==6)pet_game_reward(&p,(int)pet_random(&p,4),(int)pet_random(&p,101),1+(int)pet_random(&p,9));
  else pet_step(&p,12*60);
  assert(pet_valid(&p));
 }
 puts("20 escenarios y 50000 acciones de crianza/economia/evolucion: OK");
 return 0;
}
