#pragma once
#include <stdint.h>
#include <stddef.h>
_Static_assert(sizeof(unsigned int)==4,"Se requiere unsigned de 32 bits");
#define PET_COUNT 52
#define ITEM_COUNT 28
#define FRIEND_MAX 50
#define FLAG_SLEEP 1u
#define FLAG_DEAD 2u
#define FLAG_PAUSE 4u
#define FLAG_MUTE 8u
#define FLAG_LIGHT_OFF 16u
#define FLAG_REFUSE 32u
#define FLAG_CRY 64u
#define FLAG_BABY 128u
#define FLAG_FAST 256u
#define EGG 52
#define GRAVE 53

typedef struct { const char *name; int stage, palette, shape; } Character;
extern const Character characters[54];
typedef struct { const char *name; int price, kind, hunger, happy, weight, adult; } Item;
extern const Item items[ITEM_COUNT];
typedef struct { char name[8]; unsigned int character, friendship, sex, gifts, wrapped; } Friend;
typedef struct {
 unsigned int schema, rng, character, costume, sex, stage, generation;
 unsigned int hunger, happy, training, weight, poop, sick, tooth, flags;
 unsigned int points, physical_misses, mental_misses, hunger_empty, happy_empty, light_bad;
 unsigned int age_minutes, stage_minutes, life_minutes, neglect, poop_timer, snack_count, call_timer;
 unsigned int partner, parent1, parent2, baby_sex, baby_minutes, codes, sound, skin;
 unsigned int birthday_day, birthday_month, calendar_reward;
 unsigned int inventory[ITEM_COUNT], shop[4], shop_day, friend_count, last_reward;
 unsigned int ancestors[32], ancestor_count;
 uint64_t seen, last_time;
 int32_t clock_offset;
 char name[8], owner[16];
 Friend friends[FRIEND_MAX];
} Pet;
uint32_t pet_random(Pet *p, uint32_t limit);
void pet_new(Pet *p, uint64_t now, const char *name);
void pet_tick(Pet *p, uint64_t now);
void pet_step(Pet *p, uint64_t minute);
void pet_hatch(Pet *p);
int pet_min_weight(const Pet *p);
int pet_action(Pet *p, int action, int argument, char *message, size_t capacity);
void pet_game_reward(Pet *p, int game, int score, int bet);
void pet_next_generation(Pet *p);
int pet_valid(const Pet *p);
uint32_t pet_checksum(const void *data, size_t size);
int pet_save(const Pet *p, const char *directory);
int pet_load(Pet *p, const char *directory);

int pet_code(Pet *p, const char *code);
