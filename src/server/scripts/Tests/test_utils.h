#ifndef TEST_UTILS_H
#define TEST_UTILS_H

// Utility functions for tests
namespace Testing {
    // Create a bot as player, as well a training dummy 10 meters in front of it. Spawned in testing map (id 13)
    void PrepareCasterAndTarget(Player*& caster, Creature*& target);
    void PrepareCasterAndTarget_cleanup(Player*& caster, Creature*& target);
}
	
#endif // TEST_UTILS_H
