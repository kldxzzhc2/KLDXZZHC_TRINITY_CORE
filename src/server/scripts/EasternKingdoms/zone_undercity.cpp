/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

/* ScriptData
SDName: Undercity
SD%Complete: 95
SDComment: Quest support: 6628, 9180(post-event).
SDCategory: Undercity
EndScriptData */

/* ContentData
npc_lady_sylvanas_windrunner
npc_highborne_lamenter
npc_parqual_fintallas
EndContentData */
#include <string>

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"

/*######
## npc_lady_sylvanas_windrunner
######*/

enum Sylvanas
{
	QUEST_JOURNEY_TO_UNDERCITY = 9180,
	EMOTE_LAMENT_END = 0,
	SAY_LAMENT_END = 1,

	SOUND_CREDIT = 10896,
	ENTRY_HIGHBORNE_LAMENTER = 21628,
	ENTRY_HIGHBORNE_BUNNY = 21641,

	SPELL_HIGHBORNE_AURA = 37090,
	SPELL_SYLVANAS_CAST = 36568,
	// zhang hong chao
	SPELL_RIBBON_OF_SOULS = 37099,
	//SPELL_RIBBON_OF_SOULS = 34432, // the real one to use might be 37099

	// Combat spells
	SPELL_BLACK_ARROW = 59712,
	SPELL_FADE = 20672,
	SPELL_FADE_BLINK = 29211,
	SPELL_MULTI_SHOT = 59713,
	SPELL_SHOT = 59710,
	SPELL_SUMMON_SKELETON = 59711,
	AMBASSADOR_SUNSORROW = 16287
};

float HighborneLoc[4][3] =
{
	{ 1285.41f, 312.47f, 0.51f },
	{ 1286.96f, 310.40f, 1.00f },
	{ 1289.66f, 309.66f, 1.52f },
	{ 1292.51f, 310.50f, 1.99f },
};

#define HIGHBORNE_LOC_Y -61.00f
#define HIGHBORNE_LOC_Y_NEW -55.50f
#define GOSSIP_TEXT_GIVE_BOOK        "What will you give me?"
#define QUEST_THE_PRODIGAL_LICH_RETURNS 411

class npc_lady_sylvanas_windrunner : public CreatureScript
{
public:
	npc_lady_sylvanas_windrunner() : CreatureScript("npc_lady_sylvanas_windrunner") { }

	bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32 /*slot*/) override
	{
		if (_Quest->GetQuestId() == QUEST_JOURNEY_TO_UNDERCITY)
		{
			ENSURE_AI(npc_lady_sylvanas_windrunner::npc_lady_sylvanas_windrunnerAI, creature->AI())->LamentEvent = true;
			ENSURE_AI(npc_lady_sylvanas_windrunner::npc_lady_sylvanas_windrunnerAI, creature->AI())->DoPlaySoundToSet(creature, SOUND_CREDIT);
			ENSURE_AI(npc_lady_sylvanas_windrunner::npc_lady_sylvanas_windrunnerAI, creature->AI())->_player = player;
			creature->CastSpell(creature, SPELL_SYLVANAS_CAST, false);

			for (uint8 i = 0; i < 4; ++i)
				creature->SummonCreature(ENTRY_HIGHBORNE_LAMENTER, HighborneLoc[i][0], HighborneLoc[i][1], HIGHBORNE_LOC_Y, HighborneLoc[i][2], TEMPSUMMON_TIMED_DESPAWN, 160000);
		}

		return true;
	}

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_lady_sylvanas_windrunnerAI(creature);
	}

	struct npc_lady_sylvanas_windrunnerAI : public ScriptedAI
	{
		npc_lady_sylvanas_windrunnerAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 LamentEventTimer;
		bool LamentEvent;
		uint64 targetGUID;

		uint32 FadeTimer;
		uint32 SummonSkeletonTimer;
		uint32 BlackArrowTimer;
		uint32 ShotTimer;
		uint32 MultiShotTimer;
		bool flag;
		Player * _player;

		void Reset() override
		{
			LamentEventTimer = 5000;
			LamentEvent = false;
			targetGUID = 0;

			FadeTimer = 30000;
			SummonSkeletonTimer = 20000;
			BlackArrowTimer = 15000;
			ShotTimer = 8000;
			MultiShotTimer = 10000;
			flag = false;
			_player = NULL;
		}

		void EnterCombat(Unit* /*who*/) override { }

		void JustSummoned(Creature* summoned) override
		{
			if (summoned->GetEntry() == ENTRY_HIGHBORNE_BUNNY)
			{
				if (Creature* target = ObjectAccessor::GetCreature(*summoned, targetGUID))
				{
					target->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), me->GetPositionZ() + 15.0f, 2, 2);
					target->SetPosition(target->GetPositionX(), target->GetPositionY(), me->GetPositionZ() + 15.0f, 0.0f);
					summoned->CastSpell(target, SPELL_RIBBON_OF_SOULS, false);
				}
				summoned->SetDisableGravity(true);
				targetGUID = summoned->GetGUID();
			}
		}

		void UpdateAI(uint32 diff) override
		{
			if (LamentEvent)
			{
				if (LamentEventTimer <= diff)
				{
					DoSummon(ENTRY_HIGHBORNE_BUNNY, me, 10.0f, 3000, TEMPSUMMON_TIMED_DESPAWN);
					if (!flag) {
						Creature * sunsorrow = me->FindNearestCreature(AMBASSADOR_SUNSORROW, 40);
						if (sunsorrow && _player) {
							sunsorrow->MonsterWhisper("That, could have gone better. I have something for you.", _player);
						}
						flag = true;
					}
					LamentEventTimer = 2000;
					if (!me->HasAura(SPELL_SYLVANAS_CAST))
					{
						Talk(SAY_LAMENT_END);
						Talk(EMOTE_LAMENT_END);
						LamentEvent = false;
					}
				}
				else LamentEventTimer -= diff;
			}

			if (!UpdateVictim())
				return;

			// Combat spells

			if (FadeTimer <= diff)
			{
				DoCast(me, SPELL_FADE);
				// add a blink to simulate a stealthed movement and reappearing elsewhere
				DoCast(me, SPELL_FADE_BLINK);
				FadeTimer = 30000 + rand32() % 5000;
				// if the victim is out of melee range she cast multi shot
				if (Unit* victim = me->GetVictim())
				if (me->GetDistance(victim) > 10.0f)
					DoCast(victim, SPELL_MULTI_SHOT);
			}
			else FadeTimer -= diff;

			if (SummonSkeletonTimer <= diff)
			{
				DoCast(me, SPELL_SUMMON_SKELETON);
				SummonSkeletonTimer = 20000 + rand32() % 10000;
			}
			else SummonSkeletonTimer -= diff;

			if (BlackArrowTimer <= diff)
			{
				if (Unit* victim = me->GetVictim())
				{
					DoCast(victim, SPELL_BLACK_ARROW);
					BlackArrowTimer = 15000 + rand32() % 5000;
				}
			}
			else BlackArrowTimer -= diff;

			if (ShotTimer <= diff)
			{
				if (Unit* victim = me->GetVictim())
				{
					DoCast(victim, SPELL_SHOT);
					ShotTimer = 8000 + rand32() % 2000;
				}
			}
			else ShotTimer -= diff;

			if (MultiShotTimer <= diff)
			{
				if (Unit* victim = me->GetVictim())
				{
					DoCast(victim, SPELL_MULTI_SHOT);
					MultiShotTimer = 10000 + rand32() % 3000;
				}
			}
			else MultiShotTimer -= diff;

			DoMeleeAttackIfReady();
		}
	};
};

class npc_ambassador_sunsorrow : public CreatureScript
{
public: 
	npc_ambassador_sunsorrow() : CreatureScript("npc_ambassador_sunsorrow")  {}

	bool OnGossipHello(Player* player, Creature* creature) { 
		player->PrepareQuestMenu(creature->GetGUID());
		if (player->GetQuestStatus(QUEST_JOURNEY_TO_UNDERCITY) == QUEST_STATUS_REWARDED ) {	
				if (!player->GetItemCount(30632, true))
					player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, GOSSIP_TEXT_GIVE_BOOK, GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF);		
		}
		player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
		return true; 
	}

	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
	{
		switch (sender)
		{
		case GOSSIP_SENDER_INFO:
			if (!player->GetItemCount(30632, true)) {
				player->AddItem(30632, 1);
				player->CLOSE_GOSSIP_MENU();
				creature->MonsterSay("Here you are.", 0, player);
			}
			break;
		}
		return true;
	}

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_ambassador_sunsorrowAI(creature);
	}

	struct npc_ambassador_sunsorrowAI : public ScriptedAI
	{
		npc_ambassador_sunsorrowAI(Creature* creature) : ScriptedAI(creature) { }
	};
};

class npc_bethor_iceshard : public CreatureScript
{
public:
	npc_bethor_iceshard() : CreatureScript("npc_bethor_iceshard")  {}

	bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) { 
		if (quest->GetQuestId() == QUEST_THE_PRODIGAL_LICH_RETURNS) {
			ENSURE_AI(npc_bethor_iceshard::npc_bethor_iceshardAI, creature->AI())->_player = player;
			ENSURE_AI(npc_bethor_iceshard::npc_bethor_iceshardAI, creature->AI())->startEvent = true;
		}
		return true; 
	}

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_bethor_iceshardAI(creature);
	}

	struct npc_bethor_iceshardAI : public ScriptedAI
	{
		npc_bethor_iceshardAI(Creature* creature) : ScriptedAI(creature) { }

		void Reset() override
		{
			startEvent = false;
			_player = NULL;
			timer = 2000;
			summon = NULL;
			phase = 0;
		}

		bool startEvent;
		Player * _player;
		uint32 timer;
		Creature * summon;
		int phase;

		void UpdateAI(uint32 diff) override {
			if (startEvent) {
				switch (phase) {
				case 0:
					if (timer < diff) {
						me->CastSpell(me, 7762, false);
						phase++;
						timer = 2000;
					}
					else {
						timer -= diff;
					}
					break;
				case 1:
					if (timer < diff) {
						Position position;
						position.m_positionX = 1767.448486f;
						position.m_positionY = 60.790691f;
						position.m_positionZ = -46.320290f;
						position.m_orientation = 1.768230f;
						printf("summon...\n");
						summon = me->SummonCreature(5666, position, TEMPSUMMON_TIMED_DESPAWN, 20000);
						summon->SetWalk(true);
						position.m_positionX = 1766.795288f;
						position.m_positionY = 63.132595f;
						position.m_positionZ = -46.320072f;
						position.m_orientation = 1.803555f;
						summon->GetMotionMaster()->MovePoint(0, position, false);
						phase++;
						timer = 2000;
					}
					else {
						timer -= diff;
					}
					break;
				case 2 :
					if (timer < diff) {
						summon->MonsterSay("It has been a long time, Bethor, my friend.", 0, me);
						timer = 3000;
						phase++;
					}
					else {
						timer -= diff;
					}
					break;
				case 3 :
					if (timer < diff) {
						me->MonsterSay("When time permits, we must speak at length. For we have much to discuss.", 0, summon);
						timer = 5000;
						phase++;
					}
					else {
						timer -= diff;
					}
					break;
				case 4:
					if (timer < diff) {
						std::string str("And thank you, " + _player->GetName() + ". Without your aid I may never have found my way to the Forsaken.");
						summon->MonsterSay(str.c_str(), 0, _player);
						timer = 1000;
						phase++;
					}
					else {
						timer -= diff;
					}
					break;
				default:
					startEvent = false;
					break;
				}		
			}
		}
	};
};

/*######
## npc_highborne_lamenter
######*/

class npc_highborne_lamenter : public CreatureScript
{
public:
	npc_highborne_lamenter() : CreatureScript("npc_highborne_lamenter") { }

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_highborne_lamenterAI(creature);
	}

	struct npc_highborne_lamenterAI : public ScriptedAI
	{
		npc_highborne_lamenterAI(Creature* creature) : ScriptedAI(creature) { }

		uint32 EventMoveTimer;
		uint32 EventCastTimer;
		bool EventMove;
		bool EventCast;

		void Reset() override
		{
			EventMoveTimer = 100;
			EventCastTimer = 3000;
			EventMove = true;
			EventCast = true;
		}

		void EnterCombat(Unit* /*who*/) override { }

		void UpdateAI(uint32 diff) override
		{
			if (EventMove)
			{
				if (EventMoveTimer <= diff)
				{
					me->SetDisableGravity(true);
					//me->MonsterMoveWithSpeed(me->GetPositionX(), me->GetPositionY(), HIGHBORNE_LOC_Y_NEW, me->GetDistance(me->GetPositionX(), me->GetPositionY(), HIGHBORNE_LOC_Y_NEW) / (5000 * 0.001f));
					me->GetMotionMaster()->MoveJump(me->GetPositionX(), me->GetPositionY(), HIGHBORNE_LOC_Y_NEW, 1, 1);
					me->SetPosition(me->GetPositionX(), me->GetPositionY(), HIGHBORNE_LOC_Y_NEW, me->GetOrientation());
					EventMove = false;
				}
				else EventMoveTimer -= diff;
			}
			if (EventCast)
			{
				if (EventCastTimer <= diff)
				{
					DoCast(me, SPELL_HIGHBORNE_AURA);
					EventCast = false;
				}
				else EventCastTimer -= diff;
			}
		}
	};
};

#define DEFAULT_AURA 34426
#define ITEM_SPELL 9095
#define TEMP_CREATURE_ENTRY 37658
#define RIFT_SPAWN_ENTRY 6492
#define CONTAINMENT_COFFER 122088
#define ELM_GENERAL_PURPOSE_BUNNY 23837
#define SPELL_STUN 9032

class npc_rift_spawn : public CreatureScript
{
public:
	npc_rift_spawn() : CreatureScript("npc_rift_spawn") { }

	CreatureAI* GetAI(Creature* creature) const override
	{
		return new npc_rift_spawnAI(creature);
	}

	struct npc_rift_spawnAI : public ScriptedAI
	{
		int phase;
		uint32 timer;

		npc_rift_spawnAI(Creature* creature) : ScriptedAI(creature) { }

		void SpellHit(Unit* caster, SpellInfo const* spell) { 
			if (spell->Id == ITEM_SPELL) {
				me->RemoveAllAuras();
				DoCast(9096);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
				me->setFaction(7);
				me->AI()->AttackStart(caster);
				EnterCombat(caster);
				Talk(0);
			}
		}

		void DamageTaken(Unit* pDoneBy, uint32 &uiDamage) override
		{
			if (uiDamage > me->GetHealth() || me->HealthBelowPctDamaged(1, uiDamage))
			{
				uiDamage = 0;
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
				DoCast(SPELL_STUN);
				phase = 1;
				timer = 30000;
				me->SetEntry(TEMP_CREATURE_ENTRY);

			}
		}

		void Reset() override
		{
			me->setFaction(7);
			me->AddAura(DEFAULT_AURA, me);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
			me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
			me->SetEntry(RIFT_SPAWN_ENTRY);
			phase = 0;
		}

		void UpdateAI(uint32 diff) override
		{
			switch (phase) {
			case 1:
				if (timer < diff) {
					me->SetEntry(RIFT_SPAWN_ENTRY);
					Talk(1);
					EnterEvadeMode();
				} else {
					GameObject * obj = me->FindNearestGameObject(CONTAINMENT_COFFER, 0.1f);
					if (obj) {
						obj->Delete();
						Creature * summon = me->SummonCreature(ELM_GENERAL_PURPOSE_BUNNY, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 15000);
						summon->AI()->DoCast(9010);
						me->DespawnOrUnsummon();
						phase = 2;			
					}
					timer -= diff;
				}
				break;
			default:
				break;
			}
			DoMeleeAttackIfReady();
		}
	};
};

/*######
## npc_parqual_fintallas
######*/

enum ParqualFintallas
{
	SPELL_MARK_OF_SHAME = 6767
};

#define GOSSIP_HPF1 "Gul'dan"
#define GOSSIP_HPF2 "Kel'Thuzad"
#define GOSSIP_HPF3 "Ner'zhul"

class npc_parqual_fintallas : public CreatureScript
{
public:
	npc_parqual_fintallas() : CreatureScript("npc_parqual_fintallas") { }

	bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
	{
		player->PlayerTalkClass->ClearMenus();
		if (action == GOSSIP_ACTION_INFO_DEF + 1)
		{
			player->CLOSE_GOSSIP_MENU();
			creature->CastSpell(player, SPELL_MARK_OF_SHAME, false);
		}
		if (action == GOSSIP_ACTION_INFO_DEF + 2)
		{
			player->CLOSE_GOSSIP_MENU();
			player->AreaExploredOrEventHappens(6628);
		}
		return true;
	}

	bool OnGossipHello(Player* player, Creature* creature) override
	{
		if (creature->IsQuestGiver())
			player->PrepareQuestMenu(creature->GetGUID());

		if (player->GetQuestStatus(6628) == QUEST_STATUS_INCOMPLETE && !player->HasAura(SPELL_MARK_OF_SHAME))
		{
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HPF1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HPF2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
			player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HPF3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
			player->SEND_GOSSIP_MENU(5822, creature->GetGUID());
		}
		else
			player->SEND_GOSSIP_MENU(5821, creature->GetGUID());

		return true;
	}
};

/*######
## AddSC
######*/

void AddSC_undercity()
{
	new npc_lady_sylvanas_windrunner();
	new npc_highborne_lamenter();
	new npc_parqual_fintallas();
	new npc_ambassador_sunsorrow();
	new npc_bethor_iceshard();
	new npc_rift_spawn();
}
