#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Vehicle.h"
#include "SpellScript.h"
#include "Player.h"
/*
#define QUEST_COUNTER_ATTACK 4021
#define ARRAY_SIZE 25

int entry_array[] = {9457, 9457, 9457, 9458, 9524, 9524, 9524, 9524, 
					 9524, 9524, 9524, 9523, 9523, 9523, 9523, 9523, 
					 9524, 9523, 9524, 9523, 9524, 9523, 9524, 9524, 9524};

float position[][3] = { 
	{ -280.703f,	-1908.01f,		91.6668f },
	{ -286.384f,	-1910.99f,		91.6668f },
	{ -297.373f,	-1917.11f,		91.6746f },
	{ -293.212f,	-1912.51f,		91.6673f },
	{ -280.037f,	-1888.35f,		92.2549f },
	{ -292.107f,	-1899.54f,		91.667f  },
	{ -305.57f,		-1869.88f,		92.7754f },
	{ -289.972f,	-1882.76f,		92.5714f },
	{ -277.454f,	-1873.39f,		92.7773f },
	{ -271.581f,	-1847.51f,		93.4329f },
	{ -269.982f,	-1828.6f,		92.4754f },
	{ -279.267f,	-1827.92f,		92.3128f },
	{ -297.42f,		-1847.41f,		93.2295f },
	{ -310.607f,	-1831.89f,		95.9363f },
	{ -329.177f,	-1842.43f,		95.3891f },
	{ -324.448f,	-1860.63f,		94.3221f },
	{ -290.588f,	-1858.0f,		92.5026f },
	{ -286.103f,	-1846.18f,		92.544f  },
	{ -304.978f,	-1844.7f,		94.4432f },
	{ -308.105f,	-1859.08f,		93.8039f },
	{ -297.089f,	-1867.68f,		92.5601f },
	{ -286.988f,	-1876.47f,		92.7447f },
	{ -291.86f,		-1893.04f,		92.0213f },
	{ -298.297f,	-1846.85f,		93.3672f },
	{ -294.942f,	-1845.88f,		93.0999f }
};

Creature * c_array[ARRAY_SIZE] = {0};

class npc_deadgate : public CreatureScript
{
    public:
        npc_deadgate() : CreatureScript("npc_deadgate") { }

		bool OnQuestAccept(Player* player, Creature* creature, const Quest *_Quest) override
		{
			if (_Quest->GetQuestId() == QUEST_COUNTER_ATTACK)
			{
				ENSURE_AI(npc_deadgate::npc_deadgateAI, creature->AI())->event = true;
				ENSURE_AI(npc_deadgate::npc_deadgateAI, creature->AI())->_player = player;
				
			}
			return true;
		}

		bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) { 
			ENSURE_AI(npc_deadgate::npc_deadgateAI, creature->AI())->phase++;
			return true; 
		}
		
        struct npc_deadgateAI : public ScriptedAI
        {
            npc_deadgateAI(Creature* creature) : ScriptedAI(creature) { }

			bool event;
			Player * _player;
			int phase;
			int killCount;
			bool callFlag;
			Creature * boss;
			uint32 timer1;
			uint32 timer2;
			bool sayFlag;

			void Reset() override
			{
				event = false;
				_player = NULL;
				phase = 0;
				killCount = 0;
				callFlag = false;
				boss = NULL;
				timer1 = 1000;
				timer2 = 80000;
			}

			void SummonedCreatureDies(Creature* summoned, Unit* killer) override
			{
				if (summoned->GetEntry() == 9457 || summoned->GetEntry() == 9455) {
					if (rand32() % 4 == 0) {
						me->MonsterSay("A defender has fallen!", 0, me);
					}
				}
				if (summoned->GetEntry() == 9523 || summoned->GetEntry() == 9524) {
					if (killer->GetEntry() != 9457 && killer->GetEntry() != 9455)
						killCount++;
					if (!callFlag && killCount >= 20) {
						callFlag = true;
						me->SummonCreature(9456, Position(-296.718f, -1846.38f, 93.2334f), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 30000);
					}
				}
				if (summoned->GetEntry() == 9456) {
					killCount = 0;
					callFlag = false;
				}
			}

			void SummonedCreatureDespawn(Creature* summon) override { 
				int i = 0;
				for (i = 0; i <= ARRAY_SIZE; i++) {
					if (c_array[i] == summon)
						c_array[i] = NULL;
				}
			}

            void UpdateAI(uint32 diff) override
            {	
				if (event) {
					int i = 0;
					std::string s = "Beware, " + _player->GetName() + "!Look to the west!";
					Creature * f = NULL;
					switch (phase) {
					case 0:
						me->MonsterSay(s.c_str(), 0, _player);
						f = me->FindNearestCreature(9990, 100);
						if (f) {
							f->SetVisible(false);
						}
						me->SetFacingTo(1.0f);
						phase++;
						break;
					case 1:
						for (i = 0; i < ARRAY_SIZE; i++) {
							if (!c_array[i])
								c_array[i] = me->SummonCreature(entry_array[i],
												Position(position[i][0], position[i][1], position[i][2]),
												TEMPSUMMON_CORPSE_TIMED_DESPAWN, i < 4 ? 2000 : 20000);
							if (c_array[i]) {
								c_array[i]->GetMotionMaster()->MoveRandom(10.0f);
							}
						}
						phase++;
						break;
					case 2:
						break;
					case 3:
						for (int i = 0; i < ARRAY_SIZE; i++) {
							if (c_array[i] && c_array[i]->IsAlive()) {
								c_array[i]->DespawnOrUnsummon();
							}
						}
						f = me->FindNearestCreature(9990, 100);
						if (f) {
							f->SetVisible(true);
						}
						me->SetFacingTo(4.64258f);
						Reset();
						phase++;
					default:
						break;
					}

					if (timer1 < diff) {
						for (i = 0; i < 4; i++) {
							if (!c_array[i] || c_array[i]->isDead()) {
								c_array[i] = me->SummonCreature(entry_array[i],
												Position(position[i][0], position[i][1], position[i][2]),
												TEMPSUMMON_CORPSE_TIMED_DESPAWN, 2000);
								if (c_array[i]) {
									c_array[i]->GetMotionMaster()->MoveRandom(10.0f);
								}
							}
						}
						timer1 = 1000;
					} else {
						timer1 -= diff;
					}

					if (timer2 < diff) {
						for (i = 4; i < ARRAY_SIZE; i++) {
							if (!c_array[i] || c_array[i]->isDead()) {
								c_array[i] = me->SummonCreature(entry_array[i],
												Position(position[i][0], position[i][1], position[i][2]),
												TEMPSUMMON_CORPSE_TIMED_DESPAWN, 20000);
								if (c_array[i]) {
									c_array[i]->GetMotionMaster()->MoveRandom(10.0f);
								}
							}
						}
						timer2 = 80000;
					} else {
						timer2 -= diff;
					}
				}
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_deadgateAI(creature);
        }
};
*/
void AddSC_barrens()
{
    // new npc_deadgate();
}