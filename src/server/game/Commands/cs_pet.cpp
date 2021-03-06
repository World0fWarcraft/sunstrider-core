#include "Chat.h"
#include "Language.h"

inline Pet* GetSelectedPlayerPetOrOwn(ChatHandler* handler)
{
    if (Unit* target = handler->GetSelectedUnit())
    {
        if (target->GetTypeId() == TYPEID_PLAYER)
            return target->ToPlayer()->GetPet();
        if (target->IsPet())
            return target->ToPet();
        return nullptr;
    }
    Player* player = handler->GetSession()->GetPlayer();
    return player ? player->GetPet() : nullptr;
}

bool ChatHandler::HandleCreatePetCommand(const char* args)
{
    Player *player = m_session->GetPlayer();
    Creature *creatureTarget = GetSelectedCreature();

    if(!creatureTarget || creatureTarget->IsPet() || creatureTarget->GetTypeId() == TYPEID_PLAYER)
    {
        PSendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(creatureTarget->GetEntry());
    // Creatures with family 0 crashes the server
    if(cInfo->family == CREATURE_FAMILY_NONE)
    {
        PSendSysMessage("This creature cannot be tamed. (family id: 0).");
        SetSentErrorMessage(true);
        return false;
    }

    if(player->GetPetGUID())
    {
        PSendSysMessage("You already have a pet.");
        SetSentErrorMessage(true);
        return false;
    }

    // Everything looks OK, create new pet
    Pet* pet = new Pet(player, HUNTER_PET);
    if(!pet->CreateBaseAtCreature(creatureTarget))
    {
        delete pet;
        PSendSysMessage("Error 1");
        return false;
    }

    creatureTarget->DespawnOrUnsummon();
    creatureTarget->SetHealth(0); // just for nice GM-mode view

    pet->SetGuidValue(UNIT_FIELD_CREATEDBY, player->GetGUID());
    pet->SetCreatorGUID(player->GetGUID());
    pet->SetFaction(player->GetFaction());

    if(!pet->InitStatsForLevel(creatureTarget->GetLevel()))
    {
        TC_LOG_ERROR("command","ERROR: InitStatsForLevel() in EffectTameCreature failed! Pet deleted.");
        PSendSysMessage("Error 2");
        delete pet;
        return false;
    }

    // prepare visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL,creatureTarget->GetLevel()-1);

    pet->GetCharmInfo()->SetPetNumber(sObjectMgr->GeneratePetNumber(), true);
    // this enables pet details window (Shift+P)
    pet->AIM_Initialize();
    pet->InitPetCreateSpells();
    pet->SetHealth(pet->GetMaxHealth());

    player->GetMap()->AddToMap(pet->ToCreature(), true);

    // visual effect for levelup
    pet->SetUInt32Value(UNIT_FIELD_LEVEL, creatureTarget->GetLevel());

    pet->SetLoyaltyLevel(BEST_FRIEND);
    pet->SetPower(POWER_HAPPINESS, 1050000); //maxed

    player->SetMinion(pet, true);
    pet->SavePetToDB(PET_SAVE_AS_CURRENT);
    player->PetSpellInitialize();

    return true;
}

bool ChatHandler::HandlePetLearnCommand(const char* args)
{
    ARGS_CHECK

    Pet* pet = GetSelectedPlayerPetOrOwn(this);
    if(!pet)
    {
        PSendSysMessage("You have no pet");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 spellId = extractSpellIdFromLink((char*)args);

    if(!spellId || !sSpellMgr->GetSpellInfo(spellId))
        return false;

    // Check if pet already has it
    if(pet->HasSpell(spellId))
    {
        PSendSysMessage("Pet already has spell: %u.", spellId);
        SetSentErrorMessage(true);
        return false;
    }

    // Check if spell is valid
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if(!spellInfo || !SpellMgr::IsSpellValid(spellInfo))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN,spellId);
        SetSentErrorMessage(true);
        return false;
    }

    pet->LearnSpell(spellId);

    PSendSysMessage("Pet has learned spell %u.", spellId);
    return true;
}

bool ChatHandler::HandlePetUnlearnCommand(const char* args)
{
    ARGS_CHECK

    Pet* pet = GetSelectedPlayerPetOrOwn(this);
    if(!pet)
    {
        //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
        PSendSysMessage("You have no pet.");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 spellId = extractSpellIdFromLink((char*)args);

    if(pet->HasSpell(spellId))
        pet->RemoveSpell(spellId, false);
    else
        PSendSysMessage("Pet doesn't have that spell.");

    return true;
}

bool ChatHandler::HandlePetTpCommand(const char *args)
{
    ARGS_CHECK

    Pet* pet = GetSelectedPlayerPetOrOwn(this);
    if(!pet)
    {
        PSendSysMessage("You have no pet.");
        SetSentErrorMessage(true);
        return false;
    }

    uint32 tp = atol(args);

    pet->SetTP(tp);

    PSendSysMessage("Pet's tp changed to %u.", tp);
    return true;
}

bool ChatHandler::HandlePetHappyCommand(const char* args)
{
    Pet* pet = GetSelectedPlayerPetOrOwn(this);
    if (!pet)
    {
        //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
        PSendSysMessage("You have no pet.");
        SetSentErrorMessage(true);
        return false;
    }

    pet->SetLoyaltyLevel(BEST_FRIEND);
    pet->SetPower(POWER_HAPPINESS, 1050000); //maxed
    return true;
}

bool ChatHandler::HandlePetRenameCommand(const char* args)
{
    ARGS_CHECK

    Pet* targetPet = GetSelectedPlayerPetOrOwn(this);
    if (!targetPet)
    {
        //SendSysMessage(LANG_SELECT_PLAYER_OR_PET);
        PSendSysMessage("Select a pet or pet owner.");
        SetSentErrorMessage(true);
        return false;
    }        

    if (targetPet->getPetType() != HUNTER_PET)
    {
        SendSysMessage("Must select a hunter pet");
        SetSentErrorMessage(true);
        return false;
    }
    
    Unit *owner = targetPet->GetOwner();
    if (owner && (owner->GetTypeId() == TYPEID_PLAYER) && (owner->ToPlayer())->GetGroup())
        (owner->ToPlayer())->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);
        
    targetPet->SetName(args);
    targetPet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, time(nullptr));
    
    return true;
}
