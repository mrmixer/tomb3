#include "../tomb3/pch.h"
#include "box.h"
#include "objects.h"
#include "../specific/game.h"
#include "lot.h"
#include "../3dsystem/phd_math.h"
#include "lara.h"

void AlertNearbyGuards(ITEM_INFO* item)
{
	ITEM_INFO* target;
	CREATURE_INFO* creature;
	long dx, dy, dz, dist;

	for (int i = 0; i < 5; i++)
	{
		creature = &baddie_slots[i];

		if (creature->item_num == NO_ITEM)
			continue;

		target = &items[creature->item_num];

		if (target->room_number == item->room_number)
		{
			creature->alerted = 1;
			continue;
		}

		dx = (target->pos.x_pos - item->pos.x_pos) >> 6;
		dy = (target->pos.y_pos - item->pos.y_pos) >> 6;
		dz = (target->pos.z_pos - item->pos.z_pos) >> 6;
		dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);

		if (dist < 8000)
			creature->alerted = 1;
	}
}

void InitialiseCreature(short item_number)
{
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->object_number != ELECTRIC_CLEANER && item->object_number != SHIVA && item->object_number != TARGETS)
		item->pos.y_rot += short((GetRandomControl() - 0x4000) >> 1);

	item->collidable = 1;
	item->data = 0;
}

long CreatureActive(short item_number)
{
	ITEM_INFO* item;

	item = &items[item_number];

	if (item->status == ITEM_INVISIBLE)
	{
		if (!EnableBaddieAI(item_number, 0))
			return 0;

		item->status = ITEM_ACTIVE;
	}

	if (item->data)
		return 1;

	return 0;
}

void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info)
{
	CREATURE_INFO* creature;
	OBJECT_INFO* obj;
	ITEM_INFO* enemy;
	ROOM_INFO* r;
	FLOOR_INFO* floor;
	short* zone;
	long x, y, z;
	short pivot, ang, state;

	creature = (CREATURE_INFO*)item->data;

	if (!creature)
		return;

	obj = &objects[item->object_number];
	enemy = creature->enemy;

	if (!enemy)
	{
		enemy = lara_item;
		creature->enemy = lara_item;
	}

	if (creature->LOT.fly)
		zone = ground_zone[-1][0];
	else
		zone = ground_zone[(creature->LOT.step >> 8) - 1][flip_status];

	r = &room[item->room_number];
	floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((item->pos.x_pos - r->x) >> WALL_SHIFT)];
	item->box_number = floor->box;

	if (creature->LOT.fly)
		info->zone_number = 0x2000;
	else
		info->zone_number = zone[item->box_number];

	r = &room[enemy->room_number];
	floor = &r->floor[((enemy->pos.z_pos - r->z) >> WALL_SHIFT) + r->x_size * ((enemy->pos.x_pos - r->x) >> WALL_SHIFT)];
	enemy->box_number = floor->box;

	if (creature->LOT.fly)
		info->enemy_zone = 0x2000;
	else
		info->enemy_zone = zone[enemy->box_number];

	if (!obj->non_lot && (boxes[enemy->box_number].overlap_index & creature->LOT.block_mask ||
		creature->LOT.node[item->box_number].search_number == (creature->LOT.search_number | 0x8000)))
		info->enemy_zone |= 0x4000;

	pivot = obj->pivot_length;

	if (enemy == lara_item)
		ang = lara.move_angle;
	else
		ang = enemy->pos.y_rot;

	x = enemy->pos.x_pos + (14 * enemy->speed * phd_sin(ang) >> W2V_SHIFT) - (pivot * phd_sin(item->pos.y_rot) >> W2V_SHIFT) - item->pos.x_pos;
	y = item->pos.y_pos - enemy->pos.y_pos;
	z = enemy->pos.z_pos + (14 * enemy->speed * phd_cos(ang) >> W2V_SHIFT) - (pivot * phd_cos(item->pos.y_rot) >> W2V_SHIFT) - item->pos.z_pos;

	ang = (short)phd_atan(z, x);

	if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
		info->distance = 0x7FFFFFFF;
	else if (creature->enemy)
		info->distance = SQUARE(x) + SQUARE(z);
	else
		info->distance = 0x7FFFFFFF;

	info->angle = ang - item->pos.y_rot;
	info->enemy_facing = ang - enemy->pos.y_rot + 0x8000;

	x = abs(x);
	z = abs(z);

	if (enemy == lara_item)
	{
		state = lara_item->current_anim_state;

		if (state == AS_DUCK || state == AS_DUCKROLL || state == AS_ALL4S || state == AS_CRAWL || state == AS_ALL4TURNL || state == AS_ALL4TURNR)
			y -= 384;
	}

	if (x > z)
		info->x_angle = (short)phd_atan(x + (z >> 1), y);
	else
		info->x_angle = (short)phd_atan(z + (x >> 1), y);

	info->ahead = info->angle > -0x4000 && info->angle < 0x4000;
	info->bite = info->ahead && enemy->hit_points > 0 && abs(enemy->pos.y_pos - item->pos.y_pos) <= 512;
}

void inject_box(bool replace)
{
	INJECT(0x00416A30, AlertNearbyGuards, replace);
	INJECT(0x004142E0, InitialiseCreature, replace);
	INJECT(0x00414330, CreatureActive, replace);
	INJECT(0x00414390, CreatureAIInfo, replace);
}
