#include "pet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Character characters[54] = {
 {"Petitchi",0,0,0},{"ShiroPetitchi",0,1,0},{"Hitodetchi",1,2,1},{"Marutchi",1,3,0},{"Kinakomotchi",1,4,0},{"Kuribotchi",1,5,2},
 {"Hinatchi",2,2,3},{"Hinotamatchi",2,3,4},{"Ichigotchi",2,6,5},{"Nikatchi",2,3,0},{"Oniontchi",2,7,2},{"Propellertchi",2,8,6},
 {"Ringotchi",2,6,5},{"UFOtchi",2,9,7},{"Young Mametchi",2,2,8},{"Young Mimitchi",2,1,9},
 {"Mametchi",3,2,8},{"Pyonchitchi",3,4,9},{"Violetchi",3,7,10},{"Ginjirotchi",3,8,0},{"Kuchipatchi",3,10,11},{"Pochitchi",3,2,12},
 {"Kusatchi",3,10,13},{"Nyorotchi",3,6,14},{"Tarakotchi",3,3,11},{"Hanatchi",3,9,15},{"Kaerutchi",3,10,16},{"Takotchi",3,9,17},
 {"Kiwitchi",3,10,3},{"Masktchi",3,7,18},{"Whaletchi",3,8,19},{"ChoHimetchi",3,1,20},{"Marumimitchi",3,4,9},{"Mimitchi",3,1,9},
 {"Debatchi",3,3,11},{"Memetchi",3,3,21},{"Toratchi",3,2,22},{"Butterflytchi",3,9,23},{"Dorotchi",3,1,24},{"Kabutchi",3,10,25},
 {"Androtchi",3,11,26},{"Flowertchi",3,3,10},{"Wooltchi",3,4,27},{"Gozarutchi",3,7,18},{"Hiratchi",3,2,0},{"Zuccitchi",3,7,28},
 {"Ojitchi",4,11,29},{"Otokitchi",4,9,29},{"Oyajitchi",3,5,30},{"Pandatchi",3,1,31},{"Kigurumitchi",3,8,28},{"Nazotchi",3,6,0},
 {"Huevo",-1,1,0},{"Recuerdo",5,11,0}
};
// kind: 0 food, 1 snack, 2 toy, 3 consumable, 4 costume, 5 special.
const Item items[ITEM_COUNT] = {
 {"Comida",0,0,1,0,1,0},{"Dulce",0,1,0,1,2,0},{"Manzana",100,0,2,1,1,0},{"Hamburguesa",180,0,3,1,2,0},
 {"Pastel",600,1,0,3,3,0},{"Bistec",800,0,4,2,2,0},{"Helado",150,1,0,2,2,0},{"Palomitas",100,1,0,1,1,0},
 {"Pizza",250,0,3,1,2,0},{"Pescado",180,0,2,1,1,0},{"Pelota",200,2,0,1,0,0},{"Muñeco",1400,2,0,2,0,0},
 {"Robot",1400,2,0,2,0,0},{"Trompeta",1800,2,0,2,0,1},{"Patines",1000,2,0,2,0,1},{"Alas",1000,2,0,2,0,1},
 {"Lazo",500,2,0,1,0,1},{"Flor",1000,2,0,2,0,1},{"Planta",100,3,0,1,0,1},{"Pala",80,3,0,1,0,1},
 {"Gel de pelo",400,2,0,1,0,1},{"Miel del amor",3000,3,0,2,0,1},{"Disfraz panda",2500,4,0,3,0,1},{"Disfraz raro",0,4,0,3,0,1},
 {"OVNI",5000,2,0,4,0,1},{"Dieta",200,3,0,0,-30,1},{"Cereza",120,0,2,1,1,0},{"Bebida",100,1,0,1,1,0}
};
static uint32_t cap(int value, int maximum) { return (uint32_t)(value < 0 ? 0 : value > maximum ? maximum : value); }
uint32_t pet_random(Pet *p, uint32_t limit) { p->rng = p->rng * 1664525u + 1013904223u; return limit ? p->rng % limit : 0; }
uint32_t pet_checksum(const void *data, size_t size) { const unsigned char *s=data; uint32_t h=2166136261u; while(size--) h=(h^*s++)*16777619u; return h; }
int pet_min_weight(const Pet *p) { return p->stage == 0 ? 5 : p->stage == 1 ? 10 : p->stage == 2 ? 20 : 30; }
static void discover(Pet *p, unsigned character) { p->character=character; if(character<PET_COUNT) p->seen |= 1ull<<character; }
static void shop_refresh(Pet *p, unsigned day) { p->shop_day=day; for(int i=0;i<4;i++){do{p->shop[i]=2+pet_random(p,ITEM_COUNT-2);}while(p->shop[i]>=20&&p->shop[i]<=23);} }
void pet_new(Pet *p, uint64_t now, const char *name) {
 memset(p,0,sizeof(*p)); p->schema=1; p->rng=(uint32_t)(now^0x39a4d21bu); p->character=EGG; p->costume=255;
 p->generation=1; p->hunger=p->happy=4; p->weight=5; p->poop_timer=35; p->last_time=now; p->call_timer=180;
 p->parent1=p->parent2=p->partner=255; p->sex=pet_random(p,2); p->sound=1;p->birthday_day=1;p->birthday_month=1;
 snprintf(p->name,sizeof(p->name),"%.5s",name?name:"TAMA"); snprintf(p->owner,sizeof(p->owner),"YO"); shop_refresh(p,(unsigned)(now/86400));
}
void pet_hatch(Pet *p) { if(p->character!=EGG)return; discover(p,p->sex?1:0); p->stage=0;p->stage_minutes=0;p->points+=100; }
static void evolve(Pet *p) {
 if(p->stage==0 && p->stage_minutes>=60) { if((p->parent1==46&&p->parent2==47)||(p->parent1==47&&p->parent2==46)){discover(p,48);p->stage=3;p->stage_minutes=0;p->weight=30;return;} discover(p,p->generation==1?2:2+pet_random(p,4));p->stage=1;p->stage_minutes=0;p->weight=10;p->points+=100; }
 else if(p->stage==1 && p->stage_minutes>=1440) {
  unsigned care=p->physical_misses+p->mental_misses;
  unsigned id=(p->generation&1)?(care<2?(pet_random(p,2)?14:12):(care<5?(pet_random(p,2)?8:6):11)):(care<2?(pet_random(p,2)?15:10):(care<5?(pet_random(p,2)?7:13):9));
  discover(p,id);p->stage=2;p->stage_minutes=0;p->weight=20;p->points+=200;
 } else if(p->stage==2 && p->stage_minutes>=2880) {
  unsigned physical=p->physical_misses,mental=p->mental_misses;
  unsigned tier=physical<2 && mental<2 ? (p->training>=6?0:1): physical<3&&mental<3?1:physical<3?2:mental<3?3:4;
  unsigned id=((p->generation&1)?16:31)+tier*3+pet_random(p,3);
  if(((p->parent1==46 && p->parent2==47)||(p->parent1==47 && p->parent2==46)))id=48;
  discover(p,id);p->stage=3;p->stage_minutes=0;p->weight=30;p->points+=500;
 } else if(p->stage==3 && p->age_minutes>=12*1440 && !(p->flags&FLAG_BABY)) {discover(p,p->sex?47:46);p->stage=4;p->stage_minutes=0;}
}
void pet_next_generation(Pet *p) {
 if(!(p->flags&FLAG_BABY))return;
 if(p->ancestor_count<32)p->ancestors[p->ancestor_count++]=p->character;
 p->parent1=p->character;p->parent2=p->partner;
 p->generation++;p->sex=p->baby_sex;p->character=p->sex?1:0;p->stage=0;p->age_minutes=p->stage_minutes=0;
 p->hunger=p->happy=4;p->weight=5;p->training=p->poop=p->sick=p->tooth=0;
 p->physical_misses=p->mental_misses=p->neglect=p->hunger_empty=p->happy_empty=0;p->snack_count=0;
 p->flags &= FLAG_MUTE|FLAG_PAUSE|FLAG_FAST; p->partner=255;p->baby_minutes=0;p->poop_timer=30;p->costume=255;
 p->seen |= 1ull<<p->character;
}
void pet_step(Pet *p, uint64_t minute) {
 if(p->flags&(FLAG_PAUSE|FLAG_DEAD))return;
 p->life_minutes++; if(p->character==EGG){if(p->life_minutes>=1)pet_hatch(p);return;}
 p->age_minutes++;p->stage_minutes++;
 int clock=(int)((minute+p->clock_offset)%1440);if(clock<0)clock+=1440;
 int bedtime=p->stage<=1?20:p->stage==2?21:22;int wake=p->stage<=1?8:9;
 int sleeping=p->stage>0 && (clock>=bedtime*60 || clock<wake*60);
 if(sleeping)p->flags|=FLAG_SLEEP;else p->flags&=~FLAG_SLEEP;
 if(p->flags&FLAG_BABY){p->baby_minutes++;if(p->baby_minutes>=2880)pet_next_generation(p);}
 if(p->costume!=255 && sleeping)p->costume=255;
 if(sleeping){
  if(!(p->flags&FLAG_LIGHT_OFF)){p->light_bad++;if(p->light_bad==30)p->physical_misses++;}
  else p->light_bad=0;
  evolve(p);return;
 }
 p->light_bad=0;
 unsigned food_period=p->stage==0?3:p->stage==1?40:p->stage==4?40:60;
 unsigned happy_period=p->stage==0?4:p->stage==1?45:50;
 if(p->life_minutes%food_period==0 && p->hunger)p->hunger--;
 if(p->life_minutes%happy_period==0 && p->happy)p->happy--;
 if(!p->hunger){p->hunger_empty++;if(p->hunger_empty%15==0)p->physical_misses++;}else p->hunger_empty=0;
 if(!p->happy){p->happy_empty++;if(p->happy_empty%15==0)p->mental_misses++;}else p->happy_empty=0;
 if(p->hunger_empty>=120 || p->happy_empty>=180 || p->poop>=3){if(p->life_minutes%60==0 && p->sick<3)p->sick++;}
 if(p->poop_timer)p->poop_timer--;else {p->poop=cap((int)p->poop+1,4);p->poop_timer=p->stage?90:25;}
 if(p->call_timer)p->call_timer--;else {if(p->hunger && p->happy)p->flags|=pet_random(p,2)?FLAG_REFUSE:FLAG_CRY;p->call_timer=180+pet_random(p,181);}
 if(p->sick || !p->hunger){p->neglect++;}else if(p->neglect)p->neglect--;
 if(p->neglect>=360 || p->age_minutes>=99*1440){p->flags|=FLAG_DEAD;p->flags&=~FLAG_SLEEP;}
 evolve(p);
}
void pet_tick(Pet *p, uint64_t now) {
 if(now<p->last_time){p->last_time=now;return;}
 uint64_t elapsed=(now-p->last_time)/60;
 if(p->flags&FLAG_PAUSE){p->last_time=now;return;}
 if(elapsed>60*24*366)elapsed=60*24*366;
 for(uint64_t i=0;i<elapsed;i++){
  p->last_time+=60;pet_step(p,p->last_time/60);
  if(p->flags&FLAG_DEAD){p->last_time=now;break;}
 }
 if((unsigned)(now/86400)!=p->shop_day)shop_refresh(p,(unsigned)(now/86400));
}
static int say(char *text,size_t n,const char *value){snprintf(text,n,"%s",value);return 1;}
static int active(Pet *p,char *text,size_t n){if(p->flags&FLAG_DEAD)return say(text,n,"Tu mascota dejó un recuerdo. Puedes comenzar otro huevo.");if(p->character==EGG)return say(text,n,"El huevo está a punto de abrirse.");if(p->flags&FLAG_SLEEP)return say(text,n,"Está durmiendo. Apaga la luz para que descanse.");if(p->flags&FLAG_PAUSE)return say(text,n,"La crianza está en pausa.");return 0;}
// actions: feed, clean, medicine, praise, time-out, lights, item, buy, visit, match, code.
int pet_action(Pet *p,int action,int argument,char *text,size_t n){
 if(action==5){p->flags^=FLAG_LIGHT_OFF;return say(text,n,p->flags&FLAG_LIGHT_OFF?"Luz apagada.":"Luz encendida.");}
 if(active(p,text,n))return 0;
 if(action==0){
  if(argument<0||argument>=ITEM_COUNT)return 0;const Item *i=&items[argument];
  if(i->kind>1)return say(text,n,"Ese objeto no es comida.");
  if(argument>1&&!p->inventory[argument])return say(text,n,"No tienes ese alimento.");
  if(p->flags&FLAG_REFUSE)return say(text,n,"No quiere comer. Revisa si necesita disciplina.");
  if(i->hunger && p->hunger==4)return say(text,n,"Ya tiene la barriga llena.");
  if(argument>1)p->inventory[argument]--;
  p->hunger=cap((int)p->hunger+i->hunger,4);p->happy=cap((int)p->happy+i->happy,4);p->weight=cap((int)p->weight+i->weight,99);
  if(i->kind==1){p->snack_count++;if(p->snack_count%5==0)p->tooth=1;}
  return say(text,n,"¡Qué rico! Ha terminado de comer.");
 }
 if(action==1){if(p->poop){p->poop=0;return say(text,n,"Baño limpio. ¡Ahora está cómodo!");}if(p->poop_timer<10){p->poop_timer=90;p->happy=cap((int)p->happy+1,4);return say(text,n,"¡Llegaste a tiempo al baño!");}return say(text,n,"Todavía no necesita ir al baño.");}
 if(action==2){if(p->tooth){p->tooth=0;return say(text,n,"Se curó el dolor de muelas.");}if(p->sick){p->sick--;if(!p->sick)p->neglect=0;return say(text,n,p->sick?"Necesita otra dosis de medicina.":"¡Ya se encuentra mejor!");}return say(text,n,"Está sano, no necesita medicina.");}
 if(action==3){if(p->flags&FLAG_CRY){p->flags&=~FLAG_CRY;p->training=cap((int)p->training+1,9);p->happy=cap((int)p->happy+1,4);return say(text,n,"Se siente acompañado. Aprendió con cariño.");}return say(text,n,"Un poquito de cariño siempre viene bien.");}
 if(action==4){if(p->flags&FLAG_REFUSE){p->flags&=~FLAG_REFUSE;p->training=cap((int)p->training+1,9);return say(text,n,"Ha entendido. La disciplina aumentó.");}if(p->happy)p->happy--;p->mental_misses++;return say(text,n,"No había hecho nada malo. Se puso triste.");}
 if(action==7){if(argument<2||argument>=ITEM_COUNT)return 0;unsigned cost=(unsigned)items[argument].price;if(p->points<cost)return say(text,n,"No tienes suficientes puntos Gotchi.");if(p->inventory[argument]>=99)return say(text,n,"Ya tienes demasiados de ese objeto.");p->points-=cost;p->inventory[argument]++;return say(text,n,"¡Comprado! Está en tus objetos.");}
 if(action==6){
  if(argument<2||argument>=ITEM_COUNT||!p->inventory[argument])return say(text,n,"No tienes ese objeto.");const Item *i=&items[argument];
  if(i->kind<=1)return pet_action(p,0,argument,text,n);
  if(i->adult&&p->stage<3)return say(text,n,"Podrá usarlo cuando sea adulto.");
  p->happy=cap((int)p->happy+i->happy,4);
  if(i->kind==4){p->costume=argument==22?49:50;p->seen|=1ull<<p->costume;return say(text,n,"¡Un disfraz nuevo! Se lo quitará al dormir.");}
  if(i->kind==3){p->inventory[argument]--;if(argument==18||argument==19){unsigned reward=pet_random(p,601);p->points=cap((int)p->points+(int)reward,9999);return say(text,n,"¡Encontró una sorpresa y puntos Gotchi!");}if(argument==21){for(unsigned k=0;k<p->friend_count;k++)p->friends[k].friendship=6;return say(text,n,"La miel hace que el amor florezca.");}if(argument==25){p->weight=cap((int)p->weight-30,99);if(p->weight<(unsigned)pet_min_weight(p))p->weight=pet_min_weight(p);}}
  return say(text,n,"¡Se divirtió jugando con su objeto!");
 }
 if(action==8){
  if(argument<0||argument>=(int)p->friend_count)return 0;Friend *f=&p->friends[argument];f->friendship=cap((int)f->friendship+1,6);p->happy=cap((int)p->happy+1,4);p->seen|=1ull<<f->character;
  if(f->wrapped){f->wrapped--;if(p->inventory[10]<99)p->inventory[10]++;f->gifts++;return say(text,n,"¡Tu amigo te regaló una pelota!");}
  return say(text,n,"Pasaron un buen rato juntos. La amistad creció.");
 }
 if(action==9){
  if(p->stage<3 || p->age_minutes<7*1440)return say(text,n,"La casamentera llegará a partir de los 7 años.");
  if(p->flags&FLAG_BABY)return say(text,n,"Ya cuida de su bebé. Se quedará dos días con él.");
  if(argument<16||argument>47)return 0;
  p->partner=(unsigned)argument;p->baby_sex=pet_random(p,2);if((p->character==46&&argument==47)||(p->character==47&&argument==46))p->baby_sex=0;
  p->flags|=FLAG_BABY;p->baby_minutes=0;p->seen|=1ull<<(unsigned)argument;return say(text,n,"¡Una nueva familia! El bebé se quedará con su progenitor dos días.");
 }
 return 0;
}
void pet_game_reward(Pet *p,int game,int score,int bet){
 if(score<0)score=0;if(score>100)score=100;if(bet<1)bet=1;if(bet>9)bet=9;
 unsigned reward=(unsigned)(score*(game==0?1:4));if(game==3)reward=(unsigned)(score*bet);
 p->last_reward=reward;p->points=cap((int)p->points+(int)reward,9999);
 if(score>=40)p->happy=cap((int)p->happy+(score>=95?2:1),4);
 int loss=game==0?1+score/40:1;p->weight=(p->weight>(unsigned)loss)?p->weight-(unsigned)loss:0;
 if(p->weight<(unsigned)pet_min_weight(p))p->weight=pet_min_weight(p);
 if(score>=80){if(game==0||game==2){if(p->physical_misses)p->physical_misses--;}else if(p->mental_misses)p->mental_misses--;}
}
int pet_valid(const Pet *p){
 if(p->schema!=1||p->character>GRAVE||p->stage>4||p->sex>1||p->generation<1||p->generation>9999||p->hunger>4||p->happy>4||p->training>9||p->weight>99||p->poop>4||p->sick>3||p->friend_count>FRIEND_MAX||p->ancestor_count>32||p->points>9999||p->costume>255)return 0;
 if(!memchr(p->name,0,sizeof(p->name))||!memchr(p->owner,0,sizeof(p->owner)))return 0;
 for(unsigned i=0;i<FRIEND_MAX;i++)if(p->friends[i].character>=PET_COUNT||p->friends[i].friendship>6||p->friends[i].sex>1||!memchr(p->friends[i].name,0,sizeof(p->friends[i].name)))return 0;
 for(unsigned i=0;i<ITEM_COUNT;i++)if(p->inventory[i]>99)return 0;
 for(unsigned i=0;i<4;i++)if(p->shop[i]>=ITEM_COUNT)return 0;
 for(unsigned i=0;i<p->ancestor_count;i++)if(p->ancestors[i]>=PET_COUNT)return 0;
 return 1;
}
// Versioned local save, checked on load; a previous copy is kept.
typedef struct {uint32_t magic,version,size,hash;} SaveHeader;
int pet_save(const Pet *p,const char *directory){
 char path[1000],tmp[1000],backup[1000];snprintf(path,sizeof(path),"%s/mascota.dat",directory);snprintf(tmp,sizeof(tmp),"%s/mascota.tmp",directory);snprintf(backup,sizeof(backup),"%s/mascota.bak",directory);
 if(!pet_valid(p))return 0;SaveHeader h={0x32564354u,1,sizeof(*p),pet_checksum(p,sizeof(*p))};FILE *f=fopen(tmp,"wb");if(!f)return 0;
 int ok=fwrite(&h,1,sizeof(h),f)==sizeof(h)&&fwrite(p,1,sizeof(*p),f)==sizeof(*p);if(fclose(f))ok=0;if(!ok)return 0;
 remove(backup);rename(path,backup);if(!rename(tmp,path))return 1;rename(backup,path);return 0;
}
static int load_one(Pet *p,const char *path){FILE *f=fopen(path,"rb");if(!f)return 0;SaveHeader h;Pet q;int ok=fread(&h,1,sizeof(h),f)==sizeof(h)&&h.magic==0x32564354u&&h.version==1&&h.size==sizeof(q)&&fread(&q,1,sizeof(q),f)==sizeof(q)&&h.hash==pet_checksum(&q,sizeof(q))&&pet_valid(&q);fclose(f);if(ok)*p=q;return ok;}
int pet_load(Pet *p,const char *directory){char path[1000];snprintf(path,sizeof(path),"%s/mascota.dat",directory);if(load_one(p,path))return 1;snprintf(path,sizeof(path),"%s/mascota.bak",directory);return load_one(p,path);}

int pet_code(Pet *p,const char *code){
 const char *codes[5]={"ACBCABAC","BCBACABA","BACBCACB","CBAACABC","ABBACBCC"};
 const int prize[5]={20,21,5,22,4};
 for(unsigned i=0;i<5;i++)if(!strcmp(code,codes[i])){
  int item=prize[i];unsigned bit=1u<<i;
  if(p->codes&bit){if(p->points<(unsigned)items[item].price)return -2;p->points-=items[item].price;}
  else{p->codes|=bit;if(p->codes==31)item=23;}
  if(p->inventory[item]<99)p->inventory[item]++;return item;
 }
 return -1;
}
