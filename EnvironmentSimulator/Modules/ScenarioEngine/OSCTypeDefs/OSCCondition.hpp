/* 
 * esmini - Environment Simulator Minimalistic 
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 * 
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include "OSCCommon.hpp"
#include "CommonMini.hpp"
#include "OSCAction.hpp"
#include "Entities.hpp"
#include "OSCPosition.hpp"
#include "Parameters.hpp"

namespace scenarioengine
{
	// Forward declaration 
	class StoryBoard;
	
	class OSCCondition
	{
	public:

		typedef enum
		{
			BY_ENTITY,
			BY_STATE,
			BY_VALUE
		} ConditionType;

		typedef enum
		{
			RISING,
			FALLING,
			RISING_OR_FALLING,
			NONE,
			UNDEFINED
		} ConditionEdge;

		ConditionType base_type_;
		std::string name_;
		double delay_;
		bool evaluated_;
		bool last_result_;  // result from last evaluation
		ConditionEdge edge_;
		SE_SimulationTimer timer_;

		OSCCondition(ConditionType base_type) : base_type_(base_type), evaluated_(false), 
			last_result_(false), edge_(ConditionEdge::NONE) {}

		bool Evaluate(StoryBoard *storyBoard, double sim_time);
		virtual bool CheckCondition(StoryBoard *storyBoard, double sim_time) = 0;
		virtual void Log();
		bool CheckEdge(bool new_value, bool old_value, OSCCondition::ConditionEdge edge);
	};

	class ConditionGroup
	{
	public:
		std::vector<OSCCondition*> condition_;

		bool Evaluate(StoryBoard *storyBoard, double sim_time);
	};

	class Trigger
	{
	public:
		std::vector<ConditionGroup*> conditionGroup_;

		bool Evaluate(StoryBoard *storyBoard, double sim_time);
	};

	class TrigByEntity : public OSCCondition
	{
	public:
		struct Entity
		{
			Object *object_;
		};

		typedef enum
		{
			ANY,
			ALL
		} TriggeringEntitiesRule;

		struct TriggeringEntities
		{
			std::vector<Entity> entity_;
			TriggeringEntitiesRule rule_;
		};

		typedef enum
		{
			TIME_HEADWAY,
			DISTANCE,
			RELATIVE_DISTANCE,
			REACH_POSITION,
			TRAVELED_DISTANCE,
			END_OF_ROAD,
			TIME_TO_COLLISION,
			COLLISION,
			// not complete at all
		} EntityConditionType;

		TriggeringEntitiesRule triggering_entity_rule_;
		TriggeringEntities triggering_entities_;
		EntityConditionType type_;
		std::vector<Object*> triggered_by_entities_;

		TrigByEntity(EntityConditionType type) : OSCCondition(OSCCondition::ConditionType::BY_ENTITY), type_(type) {}

		void Print()
		{
			LOG("");
		}
	};

	class TrigByTimeHeadway : public TrigByEntity
	{
	public:
		Object *object_;
		double value_;
		bool freespace_;
		bool along_route_;
		Rule rule_;
		double hwt_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByTimeHeadway() : hwt_(0), TrigByEntity(TrigByEntity::EntityConditionType::TIME_HEADWAY) {}
		void Log();
	};

	class TrigByTimeToCollision : public TrigByEntity
	{
	public:
		Object* object_;
		OSCPosition* position_;
		double value_;
		bool freespace_;
		bool along_route_;
		Rule rule_;
		double ttc_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByTimeToCollision() : object_(0), position_(0), ttc_(0), TrigByEntity(TrigByEntity::EntityConditionType::TIME_TO_COLLISION) {}
		void Log();
	};

	class TrigByReachPosition : public TrigByEntity
	{
	public:
		OSCPosition *position_;
		double tolerance_;
		double dist_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByReachPosition() : dist_(0), TrigByEntity(TrigByEntity::EntityConditionType::REACH_POSITION) {}
		void Log();
	};

	class TrigByDistance : public TrigByEntity
	{
	public:
		OSCPosition *position_;
		double value_;
		bool freespace_;
		bool along_route_;
		Rule rule_;
		double dist_;
		
		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByDistance() : dist_(0), TrigByEntity(TrigByEntity::EntityConditionType::DISTANCE) {}
		void Log();
	};

	class TrigByTraveledDistance : public TrigByEntity
	{
	public:
		double value_;
		double odom_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time); 
		TrigByTraveledDistance() : value_(0), odom_(0), TrigByEntity(TrigByEntity::EntityConditionType::TRAVELED_DISTANCE) {}
		void Log();
	};

	class TrigByRelativeDistance : public TrigByEntity
	{
	public:
		typedef enum
		{
			LONGITUDINAL,
			LATERAL,
			INTERIAL
		} RelativeDistanceType;

		Object *object_;
		double value_;
		bool freespace_;
		RelativeDistanceType type_;
		Rule rule_;
		double rel_dist_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time); 
		TrigByRelativeDistance() : object_(0), value_(0.0), rel_dist_(0), TrigByEntity(TrigByEntity::EntityConditionType::RELATIVE_DISTANCE) {}
		void Log();
	};

	class TrigByCollision : public TrigByEntity
	{
	public:
		typedef enum
		{
			LONGITUDINAL,
			LATERAL,
			INTERIAL
		} RelativeDistanceType;

		Object* object_;
		Object::Type type_;
		Rule rule_;
		double dist_;
		typedef struct
		{
			Object* object0;
			Object* object1;
		} CollisionPair;
		std::vector<CollisionPair> collision_pair_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByCollision() : object_(0), type_(Object::Type::TYPE_NONE), dist_(0),
			TrigByEntity(TrigByEntity::EntityConditionType::COLLISION) {}
		void Log();
	};

	class TrigByEndOfRoad : public TrigByEntity
	{
	public:

		Object* object_;
		double duration_;
		Rule rule_;
		double current_duration_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByEndOfRoad() : current_duration_(0), TrigByEntity(TrigByEntity::EntityConditionType::END_OF_ROAD) {}
		void Log();

	private:
		double elapsed_time_;
	};

	class TrigByState : public OSCCondition
	{
	public:

		typedef enum
		{
			STANDBY,
			RUNNING,
			COMPLETE,
			UNDEFINED_ELEMENT_STATE,
			START_TRANSITION,  
			END_TRANSITION,
			STOP_TRANSITION,
			SKIP_TRANSITION,
			COMPLETE_TRANSITION,
			UNDEFINED_ELEMENT_TRANSITION
		} CondElementState;

		CondElementState state_;
		StoryBoardElement::ElementType element_type_;
		std::string element_name_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time); 
		TrigByState(CondElementState state, StoryBoardElement::ElementType element_type, std::string element_name) :
			OSCCondition(BY_STATE), state_(state), element_type_(element_type), element_name_(element_name) {}
		std::string CondElementState2Str(CondElementState state);
		void Log();
	};

	class TrigByValue : public OSCCondition
	{
	public:
		typedef enum
		{
			PARAMETER,
			TIME_OF_DAY,
			SIMULATION_TIME,
			UNDEFINED
		} Type;

		Type type_;
		Rule rule_;

		TrigByValue(Type type) : OSCCondition(BY_VALUE), type_(type) {}
	};

	class TrigBySimulationTime : public TrigByValue
	{
	public:
		double value_;
		double sim_time_;

		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigBySimulationTime() : sim_time_(0), TrigByValue(TrigByValue::Type::SIMULATION_TIME) {}
		void Log();
	};

	class TrigByParameter : public TrigByValue
	{
	public:
		std::string name_;
		std::string value_;
		Rule rule_;
		Parameters* parameters_;
		std::string current_value_str_;


		bool CheckCondition(StoryBoard* storyBoard, double sim_time);
		TrigByParameter() : TrigByValue(TrigByValue::Type::PARAMETER) {}
		void Log();
	};

}
