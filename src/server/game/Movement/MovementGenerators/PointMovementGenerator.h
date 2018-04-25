/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#ifndef TRINITY_POINTMOVEMENTGENERATOR_H
#define TRINITY_POINTMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

template<class T>
class PointMovementGenerator : public MovementGeneratorMedium< T, PointMovementGenerator<T> >
{
    public:
        //_o = 0 means no orientation used. Use values like 0.00001 for orientation 0.
        explicit PointMovementGenerator(uint32 _id, float _x, float _y, float _z, bool _generatePath, float _speed = 0.0f, Optional<float> finalOrient = {}, bool forceDestination = false)
            : _movementId(_id),
            _destination(_x, _y, _z), 
            _finalOrient(finalOrient),
            _speed(_speed), 
            _generatePath(_generatePath), 
            _forceDestination(forceDestination), 
            _recalculateSpeed(false) 
        { }

        bool DoInitialize(T*);
        void DoFinalize(T*);
        void DoReset(T*);
        bool DoUpdate(T*, uint32);

        void MovementInform(T*);

        void UnitSpeedChanged() override { _recalculateSpeed = true; }

        MovementGeneratorType GetMovementGeneratorType() const override;

        void GetDestination(float& x, float& y, float& z) const { x = _destination.GetPositionX(); y = _destination.GetPositionY(); z = _destination.GetPositionZ(); }

    private:
        uint32 _movementId;
        Position _destination;
        float _speed;
        bool _recalculateSpeed;

        bool _generatePath;
        bool _forceDestination;

        //! if set then unit will turn to specified _orient in provided _pos
        Optional<float> _finalOrient;

        void LaunchMove(T*);
};

class AssistanceMovementGenerator : public PointMovementGenerator<Creature>
{
    public:
        explicit AssistanceMovementGenerator(float _x, float _y, float _z) :
            PointMovementGenerator<Creature>(0, _x, _y, _z, 0.0f, true) { }

        MovementGeneratorType GetMovementGeneratorType() const override;
        void Finalize(Unit*, bool) override;
};

#endif
