#include <3ds.h>
#include <citro2d.h>
#include "pet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#define APP_DIR "sdmc:/3ds/TamaConnectionV2"
enum {HOME,HEALTH,FOOD,GAMES,PLAY,TRAIN,SHOP,BAG,FRIENDS,FRIEND_PAGE,BOOK,FAMILY,GALLERY,OPTIONS,HELP,IR_INFO,PAIR,RESET};
static Pet pet;
static C3D_RenderTarget *top,*bottom;
static C2D_TextBuf texts;
static C3D_Tex textures[64];static C2D_Image sprites[64];static Tex3DS_SubTexture subtex;
static int sprite_count,screen,selected,home_icon,return_screen=HOME,current_friend;
static int game_id,game_round,game_score,game_target,game_phase,game_lane=1,game_bet=1,game_symbols[3],game_locked;
static uint64_t phase_time,game_time,now_ms,save_time,fast_time,anim_until,notice_until,last_attention;
static char notice[256]="¡Bienvenido! Tu huevo está esperando.";
static unsigned animation,frame;
static bool save_ok=true,save_blocked,sound_ready;
static ndspWaveBuf sound_wave;static int16_t *sound_data;
static u32 bg,ink,soft,purple,lavender,mint,coral,gold,white;
static const char *home_labels[10]={"Salud","Comida","Baño","Juegos / tienda","Conexión","Disciplina","Medicina","Luz","Libreta","Atención"};
static const char *stages[6]={"Bebé","Niñez","Adolescente","Adulto","Mayor","Recuerdo"};
static uint64_t epoch(void){time_t t=time(NULL);return t<0?0:(uint64_t)t;}
static unsigned visible_character(void){return pet.costume<PET_COUNT?pet.costume:pet.character;}
static bool attention(void){return !pet.hunger||!pet.happy||pet.poop||pet.sick||pet.tooth||(pet.flags&(FLAG_REFUSE|FLAG_CRY));}
static void rect(float x,float y,float w,float h,u32 c){C2D_DrawRectSolid(x,y,0,w,h,c);}
static void rounded(float x,float y,float w,float h,float r,u32 c){rect(x+r,y,w-2*r,h,c);rect(x,y+r,w,h-2*r,c);C2D_DrawCircleSolid(x+r,y+r,0,r,c);C2D_DrawCircleSolid(x+w-r,y+r,0,r,c);C2D_DrawCircleSolid(x+r,y+h-r,0,r,c);C2D_DrawCircleSolid(x+w-r,y+h-r,0,r,c);}
static void text(const char *s,float x,float y,float size,u32 c){C2D_Text t;C2D_TextParse(&t,texts,s);C2D_DrawText(&t,C2D_WithColor,x,y,0,size,size,c);}
static void wrap(const char *s,float x,float y,float size,float width,u32 c){C2D_Text t;C2D_TextParse(&t,texts,s);C2D_DrawText(&t,C2D_WithColor|C2D_WordWrap,x,y,0,size,size,c,width);}
static void sprite(unsigned id,float cx,float cy,float scale,bool flip){
 if(id>63||!sprite_count)return;
 float x=flip?cx+16*scale:cx-16*scale;
 C2D_DrawImageAt(sprites[id],x,cy-16*scale,0,NULL,flip?-scale:scale,scale);
}
static unsigned morton(unsigned x,unsigned y){unsigned a=0;for(unsigned i=0;i<3;i++){a|=((x>>i)&1)<<(i*2);a|=((y>>i)&1)<<(i*2+1);}return a;}
static void load_sprites(void){
 FILE *f=fopen(APP_DIR "/sprites.bin","rb");if(!f)return;uint32_t header[4];if(fread(header,1,16,f)!=16||header[0]!=0x52505354||header[1]!=64||header[2]!=32||header[3]!=32){fclose(f);return;}
 subtex=(Tex3DS_SubTexture){.width=32,.height=32,.left=0,.top=1,.right=1,.bottom=0};
 unsigned char rgba[4096];for(unsigned n=0;n<64;n++){
  if(fread(rgba,1,sizeof(rgba),f)!=sizeof(rgba)||!C3D_TexInit(&textures[n],32,32,GPU_RGBA8))break;
  unsigned char *out=textures[n].data;
  for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++){unsigned src=(y*32+x)*4;unsigned dst=(((y>>3)*4+(x>>3))*64+morton(x&7,y&7))*4;out[dst]=rgba[src+3];out[dst+1]=rgba[src+2];out[dst+2]=rgba[src+1];out[dst+3]=rgba[src];}
  C3D_TexFlush(&textures[n]);C3D_TexSetFilter(&textures[n],GPU_NEAREST,GPU_NEAREST);sprites[n]=(C2D_Image){.tex=&textures[n],.subtex=&subtex};sprite_count++;
 }fclose(f);
}
static void save(void){if(save_blocked)return;save_ok=pet_save(&pet,APP_DIR);save_time=now_ms;}
static void beep(int mode){
 if(!sound_ready||!pet.sound||pet.flags&FLAG_MUTE)return;
 ndspChnWaveBufClear(0);unsigned count=2205;
 for(unsigned i=0;i<count;i++){float freq=mode==2?420:mode==1?1150:850;float gate=1-(float)i/count;sound_data[i]=(int16_t)((sinf((float)i*6.2831853f*freq/22050)>0?1:-1)*2400*gate);}
 memset(&sound_wave,0,sizeof(sound_wave));sound_wave.data_vaddr=sound_data;sound_wave.nsamples=count;DSP_FlushDataCache(sound_data,count*2);ndspChnWaveBufAdd(0,&sound_wave);
}
static void announce(const char *value,int type){snprintf(notice,sizeof(notice),"%s",value);notice_until=now_ms+7000;anim_until=now_ms+1800;animation=(unsigned)type;beep(type==2?2:0);}
static void act(int action,int argument){char result[256]={0};pet_action(&pet,action,argument,result,sizeof(result));announce(result,action==0?1:0);save();}
static void go(int destination){screen=destination;selected=0;beep(0);}
static bool available(void){if(pet.flags&FLAG_DEAD){announce("Tu mascota se fue. Su recuerdo queda en la familia.",2);return false;}if(pet.character==EGG){announce("El huevo se abrirá en aproximadamente un minuto.",0);return false;}if(pet.flags&FLAG_PAUSE){announce("La crianza está en pausa. SELECT abre los ajustes.",0);return false;}if(pet.flags&FLAG_SLEEP){announce("Está dormido. Apaga la luz para que descanse.",0);return false;}return true;}
static void keyboard(const char *hint,char *out,size_t capacity,int maxlen,bool numbers){
 SwkbdState k;swkbdInit(&k,numbers?SWKBD_TYPE_NUMPAD:SWKBD_TYPE_NORMAL,2,maxlen);swkbdSetHintText(&k,hint);swkbdSetInitialText(&k,out);swkbdSetButton(&k,SWKBD_BUTTON_LEFT,"Cancelar",false);swkbdSetButton(&k,SWKBD_BUTTON_RIGHT,"Guardar",true);
 char value[128]={0};if(swkbdInputText(&k,value,sizeof(value))==SWKBD_BUTTON_RIGHT&&value[0])snprintf(out,capacity,"%.*s",(int)capacity-1,value);
}
static void start_game(int id){
 if(!available())return;if(id>(int)pet.stage&&id!=0){announce("Ese juego se desbloqueará al crecer.",0);return;}
 if(pet.weight<=(unsigned)pet_min_weight(&pet)){announce("Necesita un poco más de peso para jugar. Prueba darle un dulce.",0);return;}
 game_id=id;game_round=game_score=game_phase=game_locked=0;game_lane=1;game_bet=1;memset(game_symbols,0,sizeof(game_symbols));screen=PLAY;phase_time=now_ms;game_time=now_ms;game_target=(int)pet_random(&pet,3);
 if(id==0)phase_time=now_ms+900+pet_random(&pet,900);
}
static void finish_game(int score,int bet){pet_game_reward(&pet,game_id,score,bet);game_score=score;game_phase=99;phase_time=now_ms;char msg[128];snprintf(msg,sizeof(msg),"¡Terminó! %u puntos Gotchi. %s",pet.last_reward,score>=70?"¡Muy bien!":"¡Sigue practicando!");announce(msg,0);save();}
static void game_input(u32 keys){
 if(keys&KEY_B){go(GAMES);return;}if(game_phase==99){if(keys&KEY_A)go(GAMES);return;}
 if(game_id==0){
  if(now_ms<phase_time)return;int picked=(keys&(KEY_LEFT|KEY_Y))?0:(keys&(KEY_UP|KEY_A))?1:(keys&(KEY_RIGHT|KEY_X))?2:-1;
  if(picked>=0||now_ms-phase_time>500){int delay=(int)(now_ms-phase_time);if(picked==game_target&&delay<=500)game_score+=100-delay/5;game_round++;beep(picked==game_target?0:2);if(game_round==3)finish_game(game_score/3,1);else{game_target=(int)pet_random(&pet,3);phase_time=now_ms+900+pet_random(&pet,900);}}
 }else if(game_id==1){
  if(game_phase==1){if(now_ms-phase_time>1000){if(game_score<=game_round*10||game_round>=8){finish_game(game_round*100/8,1);return;}game_phase=0;phase_time=now_ms;}return;}
  if(keys&KEY_A){float power=(sinf((float)(now_ms-game_time)/180)+1)*50;int chance=(int)power+(int)pet.weight/3+(int)pet.stage*3-(int)pet_random(&pet,70);if(chance>25){game_round++;game_score=1000;}else game_score=0;game_phase=1;phase_time=now_ms;beep(chance>25?0:2);}
 }else if(game_id==2){
  if(keys&KEY_LEFT&&game_lane>0)game_lane--;if(keys&KEY_RIGHT&&game_lane<2)game_lane++;if(keys&KEY_A)game_time=now_ms;
  if(now_ms-phase_time>1150){if(game_lane==game_target&&now_ms-game_time<350){game_score++;beep(0);}else beep(2);game_round++;if(game_round==20)finish_game(game_score*5,1);else{game_target=(int)pet_random(&pet,3);phase_time=now_ms;}}
 }else{
  if(game_phase==0){if(keys&KEY_UP&&game_bet<9)game_bet++;if(keys&KEY_DOWN&&game_bet>1)game_bet--;if(keys&KEY_A){if(pet.points<(unsigned)game_bet){announce("No tienes puntos suficientes para apostar.",2);return;}pet.points-=(unsigned)game_bet;game_phase=1;phase_time=now_ms;}return;}
  if(keys&KEY_A){game_symbols[game_locked]=(int)((now_ms-phase_time)/170%3);game_locked++;beep(0);if(game_locked==3){int circles=0,triangles=0;for(int i=0;i<3;i++){circles+=game_symbols[i]==2;triangles+=game_symbols[i]==1;}int multiplier=circles==3?100:circles==2?(triangles?50:30):circles==1?(triangles==2?20:triangles==1?10:8):triangles==3?5:triangles==2?3:triangles==1?1:0;finish_game(multiplier,game_bet);if(circles||triangles==3){if(pet.happy<4)pet.happy++;save();}}}
 }
}
static int inventory_map[ITEM_COUNT],inventory_count;
static void bag_entries(bool food_only){inventory_count=0;for(int i=2;i<ITEM_COUNT;i++)if(pet.inventory[i]&&(!food_only||items[i].kind<=1))inventory_map[inventory_count++]=i;}
static int list_count(void){switch(screen){case FOOD:bag_entries(true);return 2+inventory_count;case GAMES:return 6;case TRAIN:return 2;case SHOP:return 5;case BAG:bag_entries(false);return inventory_count;case FRIENDS:return 1+(int)pet.friend_count;case FRIEND_PAGE:return 5;case BOOK:return 6;case OPTIONS:return 10;case RESET:return 2;case PAIR:return 2;default:return 0;}}
static void menu_label(int index,char *out,size_t n){
 switch(screen){
 case FOOD:if(index<2)snprintf(out,n,"%s",index?"Dulce / merienda":"Comida");else{int id=inventory_map[index-2];snprintf(out,n,"%s   x%u",items[id].name,pet.inventory[id]);}break;
 case GAMES:{const char *names[]={"Jump · saltos","Bump · empujones","Heading · cabezazos","Slot · tragaperras","Objetos","Tienda"};snprintf(out,n,"%s%s",names[index],index<4&&index>(int)pet.stage?"  [al crecer]":"");break;}
 case TRAIN:snprintf(out,n,"%s",index?"Reñir / tiempo fuera":"Elogiar / consolar");break;
 case SHOP:if(index==4)snprintf(out,n,"Introducir código secreto");else{unsigned id=pet.shop[index];snprintf(out,n,"%s   %d G",items[id].name,items[id].price);}break;
 case BAG:{int id=inventory_map[index];snprintf(out,n,"%s   x%u",items[id].name,pet.inventory[id]);break;}
 case FRIENDS:if(index==0)snprintf(out,n,"Invitar amigo local");else{Friend *f=&pet.friends[index-1];snprintf(out,n,"%s · %s",f->name,characters[f->character].name);}break;
 case FRIEND_PAGE:{const char *names[]={"Visitar","Regalar un objeto","Jugar juntos","Proponer pareja","Volver"};snprintf(out,n,"%s",names[index]);break;}
 case BOOK:{const char *names[]={"Amigos locales","Familia y generaciones","Objetos","Personajes","Casamentera","Conexión infrarroja"};snprintf(out,n,"%s",names[index]);break;}
 case OPTIONS:{char buffer[128];switch(index){case 0:snprintf(buffer,sizeof(buffer),"%s crianza",pet.flags&FLAG_PAUSE?"Continuar":"Pausar");break;case 1:snprintf(buffer,sizeof(buffer),"Sonido: %s",pet.sound?"sí":"no");break;case 2:snprintf(buffer,sizeof(buffer),"Ritmo: %s",pet.flags&FLAG_FAST?"60x mientras juegas":"tiempo real");break;case 3:snprintf(buffer,sizeof(buffer),"Paleta de la interfaz: %u",pet.skin+1);break;case 4:snprintf(buffer,sizeof(buffer),"Nombre de mascota: %s",pet.name);break;case 5:snprintf(buffer,sizeof(buffer),"Tu nombre: %s",pet.owner);break;case 6:snprintf(buffer,sizeof(buffer),"Ajustar reloj");break;case 7:snprintf(buffer,sizeof(buffer),"Cumpleaños: %02u/%02u",pet.birthday_day,pet.birthday_month);break;case 8:snprintf(buffer,sizeof(buffer),"Ayuda y controles");break;default:snprintf(buffer,sizeof(buffer),"Comenzar otro huevo");}snprintf(out,n,"%s",buffer);break;}
 case RESET:snprintf(out,n,"%s",index?"Sí, empezar otro huevo":"No, conservar mascota");break;
 case PAIR:snprintf(out,n,"%s",index?"Ahora no":"Sí, formar una familia");break;
 default:out[0]=0;
 }
}
static unsigned candidate;
static void select_item(void){
 if(screen==HOME){switch(home_icon){case 0:go(HEALTH);break;case 1:go(FOOD);break;case 2:act(1,0);break;case 3:go(GAMES);break;case 4:go(IR_INFO);break;case 5:go(TRAIN);break;case 6:act(2,0);break;case 7:act(5,0);break;case 8:go(BOOK);break;case 9:announce(pet.flags&FLAG_REFUSE?"Te llamó sin motivo o rechaza comida: revisa disciplina.":pet.flags&FLAG_CRY?"Está triste o enfadado. Prueba consolarlo.":pet.sick||pet.tooth?"Necesita medicina.":!pet.hunger?"Tiene hambre.":!pet.happy?"Le vendría bien un juego.":pet.poop?"Necesita limpieza.":"Todo está bien. ¡Gracias por cuidarlo!",0);break;}return;}
 if(screen==FOOD){bag_entries(true);act(0,selected<2?selected:inventory_map[selected-2]);}
 else if(screen==GAMES){if(selected<4)start_game(selected);else go(selected==4?BAG:SHOP);}
 else if(screen==TRAIN)act(selected?4:3,0);
 else if(screen==SHOP){if(selected==4){char code[16]="";keyboard("Código de ocho letras A, B y C",code,sizeof(code),8,false);for(char *c=code;*c;c++)*c=(char)toupper((unsigned char)*c);int item=pet_code(&pet,code);if(item>=0){char msg[160];snprintf(msg,sizeof(msg),"¡Código aceptado! Recibiste: %s.",items[item].name);announce(msg,0);save();}else announce(item==-2?"No tienes puntos para repetir ese código.":"Ese código no se reconoce.",2);}else act(7,(int)pet.shop[selected]);}
 else if(screen==BAG){bag_entries(false);if(inventory_count){act(6,inventory_map[selected]);if(selected>=list_count())selected=list_count()-1;if(selected<0)selected=0;}}
 else if(screen==FRIENDS){
  if(selected==0){if(pet.friend_count>=FRIEND_MAX){announce("La libreta tiene sus 50 amigos.",0);return;}const char *names[]={"MOMO","NANA","NICO","KUMA","HANA","LUCA","LUNA","TOTO"};Friend *f=&pet.friends[pet.friend_count++];memset(f,0,sizeof(*f));snprintf(f->name,sizeof(f->name),"%.4s%u",names[pet_random(&pet,8)],pet.friend_count%10);f->character=16+pet_random(&pet,30);f->sex=pet_random(&pet,2);f->wrapped=1;pet.seen|=1ull<<f->character;announce("Un vecino virtual se unió a tus amigos locales.",0);save();}
  else{current_friend=selected-1;go(FRIEND_PAGE);}
 }else if(screen==FRIEND_PAGE){Friend *f=&pet.friends[current_friend];
  if(selected==0)act(8,current_friend);
  else if(selected==1){int gift=-1;for(int i=2;i<ITEM_COUNT;i++)if(pet.inventory[i]){gift=i;break;}if(gift<0)announce("Primero compra un regalo en la tienda.",0);else{pet.inventory[gift]--;if(f->friendship<6)f->friendship++;char msg[160];snprintf(msg,sizeof(msg),"Le regalaste %s a %s.",items[gift].name,f->name);announce(msg,0);save();}}
  else if(selected==2){if(f->friendship<6)f->friendship++;pet_game_reward(&pet,0,60,1);announce("Jugaron juntos y ganaste puntos. ¡Qué buena amistad!",0);save();}
  else if(selected==3){if(f->sex==pet.sex||f->friendship<6||pet.stage<3||pet.age_minutes<5*1440)announce("Necesitan ser adultos, tener buena amistad y distinto género.",0);else if(pet.flags&FLAG_BABY)announce("Ya está cuidando de su bebé.",0);else{pet.partner=f->character;pet.baby_sex=pet_random(&pet,2);pet.baby_minutes=0;pet.flags|=FLAG_BABY;announce("¡Formaron una familia! El bebé ya está aquí.",0);save();}}
  else go(FRIENDS);
 }else if(screen==BOOK){int destinations[]={FRIENDS,FAMILY,BAG,GALLERY,PAIR,IR_INFO};if(selected==4){candidate=16+pet_random(&pet,30);if(pet.stage==4)candidate=pet.sex?46:47;}go(destinations[selected]);}
 else if(screen==PAIR){if(selected==0){act(9,(int)candidate);}go(HOME);}
 else if(screen==OPTIONS){
  if(selected==0){pet.flags^=FLAG_PAUSE;pet.last_time=epoch();announce(pet.flags&FLAG_PAUSE?"Crianza pausada. Puedes descansar tranquilo.":"La crianza continúa.",0);}
  if(selected==1)pet.sound=!pet.sound;
  if(selected==2){pet.flags^=FLAG_FAST;pet.last_time=epoch();fast_time=epoch();announce(pet.flags&FLAG_FAST?"Ritmo rápido 60x, solo mientras la app está abierta.":"Ritmo clásico en tiempo real.",0);}
  if(selected==3)pet.skin=(pet.skin+1)%4;
  if(selected==4){char name[8];snprintf(name,sizeof(name),"%s",pet.name);keyboard("Nombre de tu mascota (5 letras)",name,sizeof(name),5,false);snprintf(pet.name,sizeof(pet.name),"%.5s",name);}
  if(selected==5)keyboard("Tu nombre",pet.owner,sizeof(pet.owner),10,false);
  if(selected==6){char value[8]="";keyboard("Hora y minutos: HHMM (ejemplo 1830)",value,sizeof(value),4,true);if(strlen(value)==4){int h=(value[0]-'0')*10+value[1]-'0',m=(value[2]-'0')*10+value[3]-'0';if(h<24&&m<60&&h>=0&&m>=0)pet.clock_offset=h*60+m-(int)(epoch()/60%1440);else announce("La hora no es válida.",2);}}
  if(selected==7){char value[8]="";keyboard("Tu cumpleaños: DDMM (ejemplo 2107)",value,sizeof(value),4,true);if(strlen(value)==4){int d=(value[0]-'0')*10+value[1]-'0',m=(value[2]-'0')*10+value[3]-'0';if(d>=1&&d<=31&&m>=1&&m<=12){pet.birthday_day=(unsigned)d;pet.birthday_month=(unsigned)m;}}}
  if(selected==8)go(HELP);if(selected==9)go(RESET);save();
 }else if(screen==RESET){if(selected){uint64_t seen=pet.seen;pet_new(&pet,epoch(),"TAMA");pet.seen=seen;save_blocked=false;save();announce("Un huevo nuevo. El álbum de personajes se conserva.",0);}go(HOME);}
}
static void back(void){switch(screen){case FOOD:case HEALTH:case TRAIN:case IR_INFO:case BOOK:screen=HOME;break;case SHOP:case BAG:screen=GAMES;break;case FRIENDS:case FAMILY:case GALLERY:case PAIR:screen=BOOK;break;case FRIEND_PAGE:screen=FRIENDS;break;case HELP:case RESET:screen=OPTIONS;break;default:screen=HOME;}selected=0;}
static void hearts(unsigned value,float x,float y){for(unsigned i=0;i<4;i++){if(i<value)sprite(60,x+i*16,y,0.43f,false);else rounded(x-5+i*16,y-4,9,8,2,soft);}}
static void draw_game(void){
 if(game_id==0){for(int i=0;i<3;i++)C2D_DrawCircleSolid(161+i*39,86,0,8,now_ms>=phase_time&&i==game_target?coral:soft);sprite(visible_character(),200,137-(game_phase==99?18:0),1.2f,false);}
 else if(game_id==1){float power=(sinf((float)(now_ms-game_time)/180)+1)*50;rect(144,78,112,9,soft);rect(144,78,power*1.12f,9,coral);sprite(visible_character(),170+(game_phase==1&&game_score?10:0),135,1.1f,false);sprite(16+(unsigned)(game_round*3)%30,234,135,1.1f,true);}
 else if(game_id==2){float progress=(float)(now_ms-phase_time)/1150;if(progress>1)progress=1;C2D_DrawCircleSolid(160+game_target*40,78+progress*57,0,6,coral);sprite(visible_character(),160+game_lane*40,145-(now_ms-game_time<350?17:0),0.9f,false);}
 else{for(int i=0;i<3;i++){int symbol=i<game_locked?game_symbols[i]:(int)((now_ms-phase_time)/170%3);rounded(145+i*38,99,32,34,3,white);text(symbol==2?"O":symbol==1?"^":"X",151+i*38,105,0.65f,symbol==2?coral:ink);}if(game_phase==0)text("APUESTA",163,76,0.4f,ink);}
}
/* Pixel scenery is drawn at integer coordinates: no simulated plastic shell. */
static void cloud(int x,int y,u32 c){rect(x,y+4,36,8,c);rect(x+6,y,23,16,c);}
static void flower(int x,int y,u32 c){rect(x+3,y+5,2,9,C2D_Color32(77,143,104,255));rect(x,y+2,8,4,c);rect(x+2,y,4,8,c);rect(x+3,y+3,2,2,gold);}
static void room(void){
 bool night=(pet.flags&FLAG_LIGHT_OFF)!=0;
 u32 wall=night?C2D_Color32(48,56,90,255):C2D_Color32(246,207,183,255);
 u32 stripe=night?C2D_Color32(55,64,99,255):C2D_Color32(250,220,194,255);
 rect(0,30,400,155,wall);
 for(int x=0;x<400;x+=20){rect(x,30,9,155,stripe);for(int y=42;y<171;y+=28){rect(x+12,y,3,3,C2D_Color32(221,162,161,night?35:140));rect(x+11,y+1,5,1,C2D_Color32(221,162,161,night?35:140));}}
 /* Window, distant hills, frame and little curtains. */
 rect(144,48,112,79,purple);rect(148,52,104,71,night?C2D_Color32(31,52,90,255):C2D_Color32(147,221,236,255));
 if(night){rect(226,60,12,12,gold);rect(221,57,12,12,C2D_Color32(31,52,90,255));for(int i=0;i<5;i++)rect(158+i*17,60+(i%3)*9,2,2,white);}
 else{rect(224,59,13,13,gold);cloud(158,65,white);cloud(209,84,C2D_Color32(225,250,245,255));}
 rect(148,105,104,18,C2D_Color32(120,181,132,255));rect(148,114,104,9,C2D_Color32(92,162,124,255));
 rect(198,52,4,71,purple);rect(148,88,104,4,purple);rect(138,124,124,6,C2D_Color32(177,111,117,255));
 rect(134,45,132,5,purple);for(int i=0;i<3;i++){rect(137+i*7,50,6,43-i*8,coral);rect(244+i*7,50,6,27+i*8,coral);}
 /* Shelves and a plant make this a room rather than an empty backdrop. */
 rect(29,86,67,6,C2D_Color32(177,111,117,255));rect(36,67,9,19,mint);rect(47,62,9,24,lavender);rect(58,70,8,16,gold);sprite(60,82,76,0.48f,false);
 rect(309,142,38,35,C2D_Color32(177,111,117,255));rect(305,137,46,7,gold);rect(315,148,26,3,C2D_Color32(236,185,145,255));
 rect(322,110,13,27,C2D_Color32(95,160,117,255));rect(311,111,15,10,C2D_Color32(111,181,126,255));rect(333,100,15,12,C2D_Color32(111,181,126,255));flower(320,100,coral);
 rect(0,178,400,7,C2D_Color32(177,111,117,255));rect(0,185,400,55,night?C2D_Color32(88,76,106,255):C2D_Color32(220,164,126,255));
 for(int y=188;y<240;y+=18){rect(0,y,400,2,C2D_Color32(192,136,113,255));for(int x=(y/18%2)*40;x<400;x+=80)rect(x,y,2,18,C2D_Color32(192,136,113,255));}
 C2D_DrawEllipseSolid(96,181,0,208,44,C2D_Color32(161,105,133,255));C2D_DrawEllipseSolid(101,183,0,198,38,lavender);C2D_DrawEllipseSolid(111,187,0,178,29,C2D_Color32(228,198,221,255));
 for(int x=131;x<280;x+=18)rect(x,199,6,2,C2D_Color32(199,162,202,255));
}
static void draw_top(void){
 C2D_TargetClear(top,bg);C2D_SceneBegin(top);room();
 rect(0,0,400,30,purple);rect(0,29,400,2,gold);
 char value[96];snprintf(value,sizeof(value),"%s / %s",pet.name,characters[pet.character].name);text(value,10,6,0.43f,white);
 int minute=((int)(epoch()/60%1440)+pet.clock_offset)%1440;minute=(minute+1440)%1440;
 snprintf(value,sizeof(value),"%02d:%02d",minute/60,minute%60);text(value,341,5,0.52f,white);
 if(screen==PLAY){rounded(114,56,172,111,8,C2D_Color32(244,232,205,245));draw_game();}
 else if(pet.flags&FLAG_DEAD){rounded(142,137,116,44,7,lavender);text("Gracias por todo",149,148,0.39f,purple);sprite(visible_character(),200,114,1.6f,false);}
 else{
  float walk=pet.flags&FLAG_SLEEP?0:sinf((float)now_ms/2100)*38;
  float bob=pet.flags&FLAG_SLEEP?0:(float)((now_ms/280)%2)*3;
  C2D_DrawEllipseSolid(169+walk,188,0,62,12,C2D_Color32(129,92,135,95));
  sprite(visible_character(),200+walk,165-bob,pet.character==EGG?2.0f:2.5f,(now_ms/6500)%2!=0);
  if(pet.flags&FLAG_BABY)sprite(pet.baby_sex?1:0,253+walk,183,0.9f,false);
  if(pet.flags&FLAG_SLEEP){text("z",237,115,0.8f,white);text("z",254,104,0.5f,white);}
  if(pet.sick||pet.tooth){rounded(249+walk,121,18,24,4,white);text("!",254+walk,122,0.7f,coral);}
  for(unsigned i=0;i<pet.poop;i++)sprite(61,109+i*23,191,0.7f,false);
  if(now_ms<anim_until){if(animation==1)sprite(56,153+walk,166,0.9f,false);else sprite(60,236+walk,122,0.7f,false);}
 }
 rounded(8,215,384,20,5,C2D_Color32(74,49,104,238));
 text("HAMBRE",16,220,0.25f,white);hearts(pet.hunger,66,226);
 text("FELIZ",130,220,0.25f,white);hearts(pet.happy,168,226);
 sprite(63,243,225,0.36f,false);snprintf(value,sizeof(value),"%u",pet.points);text(value,253,218,0.35f,white);
 snprintf(value,sizeof(value),"G%u / %ud",pet.generation,pet.age_minutes/1440);text(value,324,220,0.28f,white);
 if(pet.flags&FLAG_PAUSE){rounded(11,39,65,20,4,purple);text("PAUSA",20,43,0.36f,white);}
 else if(pet.flags&FLAG_FAST){rounded(11,39,52,20,4,purple);text("60x",21,43,0.36f,white);}
 else if(attention()&&!(pet.flags&FLAG_SLEEP)){rounded(294,40,96,22,4,coral);text("¡Te necesita!",302,45,0.34f,white);}
}
static const char *screen_title(void){switch(screen){case HEALTH:return "Cómo está tu mascota";case FOOD:return "Hora de comer";case GAMES:return "Juegos, objetos y tienda";case PLAY:return game_id==0?"Jump":game_id==1?"Bump":game_id==2?"Heading":"Slot";case TRAIN:return "Disciplina y cariño";case SHOP:return "Tienda Gotchi";case BAG:return "Tus objetos";case FRIENDS:return "Amigos locales";case FRIEND_PAGE:return "Una visita entre amigos";case BOOK:return "Tu libreta";case FAMILY:return "Tu familia";case GALLERY:return "Álbum de personajes";case OPTIONS:return "Ajustes";case HELP:return "Cómo jugar";case IR_INFO:return "Conexión";case PAIR:return "La casamentera";case RESET:return "¿Comenzar otro huevo?";default:return "¿Qué hacemos hoy?";}}
static void draw_bottom(void){
 C2D_TargetClear(bottom,lavender);C2D_SceneBegin(bottom);
 for(int y=0;y<240;y+=12)for(int x=0;x<320;x+=12)if((x/12+y/12)%2)rect(x,y,12,12,C2D_Color32(191,170,218,255));
 rounded(5,33,310,193,10,C2D_Color32(244,224,211,255));
 rect(0,0,320,31,purple);rect(0,30,320,3,gold);text(screen_title(),12,7,0.51f,white);
 if(screen==HOME){
  for(int i=0;i<10;i++){int x=9+(i%5)*62,y=42+(i/5)*58;rounded(x,y+3,55,51,6,C2D_Color32(175,132,162,255));rounded(x,y,55,51,6,home_icon==i?purple:C2D_Color32(252,239,217,255));rect(x+7,y+3,41,2,home_icon==i?lavender:white);
   unsigned icons[]={60,56,61,63,51,60,62,53,49,60};sprite(icons[i],x+27,y+18,0.7f,false);
   const char *shorts[]={"Salud","Comida","Baño","Juegos","Conectar","Educar","Medicina","Luz","Libreta","Aviso"};text(shorts[i],x+4,y+35,0.29f,home_icon==i?white:purple);
  }
  rounded(13,158,294,25,5,lavender);text(home_labels[home_icon],23,163,0.43f,purple);
  for(int i=0;i<3;i++){int x=18+i*97;rounded(x,193,90,29,5,C2D_Color32(175,132,162,255));rounded(x,190,90,29,5,purple);text(i==0?"Elegir":i==1?"Aceptar":"Volver",x+13,198,0.38f,white);}
 }else if(screen==HEALTH){
  char v[120];snprintf(v,sizeof(v),"%s · %s · generación %u",pet.name,stages[pet.stage],pet.generation);text(v,17,42,0.44f,ink);
  text("Hambre",18,76,0.43f,ink);hearts(pet.hunger,160,88);text("Felicidad",18,105,0.43f,ink);hearts(pet.happy,160,117);
  text("Educación",18,136,0.43f,ink);for(unsigned i=0;i<9;i++)rect(160+i*14,140,10,12,i<pet.training?purple:soft);
  snprintf(v,sizeof(v),"Salud: %s%s",pet.sick?"enfermo":"bien",pet.tooth?" · dolor de muelas":"");text(v,18,171,0.4f,ink);snprintf(v,sizeof(v),"Dueño: %s · %02u/%02u",pet.owner,pet.birthday_day,pet.birthday_month);text(v,18,198,0.38f,ink);
 }else if(screen==GALLERY){
  int first=selected/8*8;for(int i=0;i<8&&first+i<PET_COUNT;i++){int id=first+i,x=6+(i%4)*79,y=35+(i/4)*82;rounded(x,y,74,77,7,id==selected?lavender:C2D_Color32(252,239,217,255));sprite((unsigned)id,x+37,y+27,1.3f,false);char name[20];snprintf(name,sizeof(name),"%.12s",characters[id].name);text(name,x+4,y+54,0.29f,purple);if(!(pet.seen&(1ull<<id)))text("por conocer",x+8,y+67,0.23f,ink);}
  text("Cruceta: explorar · B: volver",17,216,0.35f,ink);
 }else if(screen==FAMILY){
  char line[160];snprintf(line,sizeof(line),"Generación actual: %u",pet.generation);text(line,17,43,0.47f,ink);
  if(pet.ancestor_count){unsigned first=pet.ancestor_count>5?pet.ancestor_count-5:0;for(unsigned i=first;i<pet.ancestor_count;i++){snprintf(line,sizeof(line),"G%u  %s",i+1,characters[pet.ancestors[i]].name);text(line,20,80+(i-first)*23,0.43f,purple);}}
  else wrap("Tu historia está empezando. Aquí quedará el recuerdo de cada progenitor.",20,88,0.46f,280,ink);
  if(pet.flags&FLAG_BABY)text("¡El bebé está con su progenitor!",18,203,0.39f,coral);
 }else if(screen==HELP){wrap("Cruceta: elegir. A: aceptar. B: volver. X: reloj. SELECT: ajustes. START: guardar y salir.\n\nCuida el hambre y la felicidad, limpia el baño, cura las enfermedades y apaga la luz al dormir. Elogia si llora y disciplina si rechaza sin motivo. Los juegos dan puntos Gotchi.\n\nLa crianza sigue al cerrar la app, salvo que la pauses. El ritmo 60x solo acelera con la app abierta.",17,42,0.39f,285,ink);}
 else if(screen==IR_INFO){wrap("El infrarrojo queda para una siguiente versión. Esta app no se anuncia como compatible con el juguete físico.\n\nMientras tanto, puedes invitar vecinos virtuales y jugar con tus amigos locales desde la libreta.",18,48,0.47f,280,ink);}
 else if(screen==PLAY){
  char msg[160];snprintf(msg,sizeof(msg),"Ronda %d · %d",game_round+1,game_id==1?8:game_id==2?20:3);text(msg,18,44,0.49f,ink);
  if(game_phase==99){snprintf(msg,sizeof(msg),"Ganaste %u puntos",pet.last_reward);text(msg,25,88,0.65f,purple);wrap(notice,24,125,0.44f,270,ink);text("A: continuar · B: volver",29,204,0.4f,purple);}
  else if(game_id==0){wrap("Cuando se encienda el círculo, pulsa la dirección correspondiente en menos de medio segundo.",20,83,0.5f,280,ink);text("Izquierda / arriba / derecha",22,168,0.43f,purple);}
  else if(game_id==1){wrap("A: detén la barra cuando esté llena. Tu peso y la fuerza ayudan a empujar al rival.",20,84,0.5f,280,ink);}
  else if(game_id==2){wrap("Izquierda y derecha para moverte. A para saltar y golpear la pelota antes de que caiga.",20,84,0.5f,280,ink);}
  else{snprintf(msg,sizeof(msg),"Apuesta: %d G",game_bet);text(msg,24,90,0.62f,purple);wrap(game_phase==0?"Arriba / abajo: elegir de 1 a 9 puntos. A: comenzar.":"A: detén cada casilla. O es la mejor figura. Tres O dan el premio mayor.",22,132,0.48f,275,ink);}
 }else{
  int length=list_count(),first=selected/5*5;
  if(screen==PAIR){sprite(candidate,260,52,1.25f,false);text(characters[candidate].name,20,48,0.47f,ink);first=0;}
  for(int row=0;row<5&&first+row<length;row++){int index=first+row,y=screen==PAIR?98+row*35:41+row*31;char name[180];menu_label(index,name,sizeof(name));rounded(12,(float)y,296,28,6,index==selected?lavender:C2D_Color32(252,239,217,255));text(name,23,y+5,0.43f,purple);}
  if(!length)wrap("Todavía no tienes objetos. Visita la tienda para conseguir alguno.",22,80,0.5f,270,ink);
  if(screen==SHOP){char coins[64];snprintf(coins,sizeof(coins),"%u puntos Gotchi",pet.points);text(coins,18,207,0.38f,ink);}
  if(screen==FRIEND_PAGE){Friend *f=&pet.friends[current_friend];char v[96];snprintf(v,sizeof(v),"%s · amistad %u/6",f->name,f->friendship);text(v,18,206,0.38f,ink);}
  if(length>5){char page[40];snprintf(page,sizeof(page),"%d / %d",selected+1,length);text(page,252,216,0.32f,ink);}
 }
 if(screen!=HOME&&screen!=PLAY)text("A: elegir   B: volver   SELECT: ajustes",14,230,0.29f,ink);
 if(now_ms<notice_until&&screen!=PLAY){rounded(8,170,304,53,7,C2D_Color32(255,252,239,248));wrap(notice,17,178,0.37f,286,purple);}
 if(!save_ok)text("No pude guardar. Revisa la SD.",9,2,0.32f,coral);
}
static void draw(void){
 static const unsigned char palettes[4][6]={{92,64,139,205,187,237},{39,103,124,164,217,216},{145,65,105,239,179,198},{121,81,49,239,209,149}};
 const unsigned char *palette=palettes[pet.skin%4];purple=C2D_Color32(palette[0],palette[1],palette[2],255);lavender=C2D_Color32(palette[3],palette[4],palette[5],255);
 C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(texts);draw_top();draw_bottom();C3D_FrameEnd(0);frame++;}
static void suspend_hook(APT_HookType hook,void *unused){(void)unused;if(hook==APTHOOK_ONSLEEP||hook==APTHOOK_ONSUSPEND)save();}
int main(int argc,char **argv){
 (void)argc;(void)argv;gfxInitDefault();if(!C3D_Init(C3D_DEFAULT_CMDBUF_SIZE)||!C2D_Init(C2D_DEFAULT_MAX_OBJECTS)){gfxExit();return 1;}C2D_Prepare();
 top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);bottom=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);texts=C2D_TextBufNew(8192);
 if(!top||!bottom||!texts){C2D_Fini();C3D_Fini();gfxExit();return 1;}
 bg=C2D_Color32(249,245,255,255);ink=C2D_Color32(51,47,76,255);soft=C2D_Color32(213,213,221,255);purple=C2D_Color32(92,64,139,255);lavender=C2D_Color32(205,187,237,255);mint=C2D_Color32(230,247,219,255);coral=C2D_Color32(222,101,118,255);gold=C2D_Color32(237,186,76,255);white=C2D_Color32(255,255,255,255);
 mkdir(APP_DIR,0777);load_sprites();now_ms=osGetTime();fast_time=epoch();
 if(!pet_load(&pet,APP_DIR)){struct stat old;if(!stat(APP_DIR "/mascota.dat",&old)||!stat(APP_DIR "/mascota.bak",&old)){save_blocked=true;snprintf(notice,sizeof(notice),"La partida no se pudo leer. No se sobrescribirá. SELECT permite comenzar otro huevo.");}pet_new(&pet,epoch(),"TAMA");}
 else snprintf(notice,sizeof(notice),"¡Hola de nuevo, %s! Tu mascota te esperaba.",pet.owner);
 notice_until=now_ms+7000;pet_tick(&pet,epoch());
 if(R_SUCCEEDED(ndspInit())){sound_data=linearAlloc(2205*2);if(sound_data){sound_ready=true;ndspChnSetRate(0,22050);ndspChnSetFormat(0,NDSP_FORMAT_MONO_PCM16);float mix[12]={0};mix[0]=mix[1]=0.25f;ndspChnSetMix(0,mix);}else ndspExit();}
 aptHookCookie hook;aptHook(&hook,suspend_hook,NULL);save_time=now_ms;
 unsigned previous_generation=pet.generation;bool quit=false;
 while(aptMainLoop()&&!quit){now_ms=osGetTime();uint64_t t=epoch();unsigned previous_character=pet.character;
  if(pet.flags&FLAG_FAST){uint64_t elapsed=t>=fast_time?t-fast_time:0;if(elapsed>600)elapsed=600;for(uint64_t i=0;i<elapsed;i++)pet_step(&pet,t/60);pet.last_time=t;fast_time=t;}
  else{pet_tick(&pet,t);fast_time=t;}
  if(previous_character!=pet.character){announce(pet.character<2?"¡Nació tu mascota!": "¡Tu mascota ha crecido!",0);save();}
  if(previous_generation!=pet.generation){previous_generation=pet.generation;announce("¡Comienza una nueva generación! Puedes elegir su nombre en ajustes.",0);save();}
  time_t stamp=(time_t)t;struct tm *date=localtime(&stamp);unsigned day=(unsigned)(t/86400);
  if(date&&pet.calendar_reward!=day&&((date->tm_mon+1==(int)pet.birthday_month&&date->tm_mday==(int)pet.birthday_day)||(date->tm_mon==11&&date->tm_mday==25))){pet.calendar_reward=day;if(pet.inventory[4]<99)pet.inventory[4]++;announce("¡Día especial! Recibiste un pastel de regalo.",0);save();}
  if(attention()&&!(pet.flags&(FLAG_SLEEP|FLAG_DEAD|FLAG_PAUSE))&&now_ms-last_attention>60000){beep(2);last_attention=now_ms;}
  if(now_ms-save_time>30000)save();
  hidScanInput();u32 keys=hidKeysDown();
  if(keys&KEY_START){quit=true;continue;}
  if(keys&KEY_SELECT){return_screen=screen;go(OPTIONS);}
  else if(keys&KEY_X&&screen!=PLAY){char clock_text[80];int m=((int)(t/60%1440)+pet.clock_offset+1440)%1440;snprintf(clock_text,sizeof(clock_text),"Son las %02d:%02d. SELECT permite ajustar el reloj.",m/60,m%60);announce(clock_text,0);}
  if(screen==PLAY)game_input(keys);
  else if(screen==GALLERY){if(keys&KEY_LEFT)selected=(selected+PET_COUNT-1)%PET_COUNT;if(keys&KEY_RIGHT)selected=(selected+1)%PET_COUNT;if(keys&KEY_UP)selected=(selected+PET_COUNT-4)%PET_COUNT;if(keys&KEY_DOWN)selected=(selected+4)%PET_COUNT;if(keys&KEY_A)announce(characters[selected].name,0);if(keys&KEY_B)back();}
  else{
   int length=list_count();if(screen==HOME){if(keys&KEY_LEFT)home_icon=(home_icon+9)%10;if(keys&KEY_RIGHT)home_icon=(home_icon+1)%10;if(keys&KEY_UP||keys&KEY_DOWN)home_icon=(home_icon+5)%10;}
   else if(length){if(keys&KEY_UP)selected=(selected+length-1)%length;if(keys&KEY_DOWN)selected=(selected+1)%length;}
   if(keys&KEY_A)select_item();if(keys&KEY_B){if(screen==OPTIONS){screen=return_screen;selected=0;}else back();}
   if(keys&KEY_TOUCH){touchPosition touch;hidTouchRead(&touch);
    if(screen==HOME){if(touch.py>=42&&touch.py<152){int col=(touch.px-9)/62,row=(touch.py-42)/58;if(col>=0&&col<5&&row>=0&&row<2){home_icon=row*5+col;select_item();}}
     else if(touch.py>=187&&touch.py<227){if(touch.px<115)home_icon=(home_icon+1)%10;else if(touch.px<205)select_item();else back();}}
    else if(length){int origin=screen==PAIR?98:41,step=screen==PAIR?35:31;if(touch.py>=origin&&touch.py<origin+step*5){int i=selected/5*5+(touch.py-origin)/step;if(i<length){selected=i;select_item();}}}
   }
  }
  draw();
 }
 save();aptUnhook(&hook);if(sound_ready){ndspChnWaveBufClear(0);linearFree(sound_data);ndspExit();}
 for(int i=0;i<sprite_count;i++)C3D_TexDelete(&textures[i]);C2D_TextBufDelete(texts);C2D_Fini();C3D_Fini();gfxExit();return 0;
}
