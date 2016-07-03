#pragma once
#include <string>
#include <atomic>
#include <vector>

#include "MovingPlan_Base.h"
#include "MovingPlan.h"
#include "sensor.h"

#define DIST_ULTRASONIC23 150			//distance between ultrasonic 2 and 3
/************************************************************************/
/* MovingTask_Base                                                      */
/************************************************************************/
enum Task_State :int
{
    TS_Stoped = 0,
    TS_Playing,
    TS_Pausing,
};

typedef struct{
	MoveType moveType;
	int		 param1;
	int		 param2;
	DWORD	 time;				//ms
}Action;

class MovingTask_Base :public MovingTask
{
public:
    MovingTask_Base(MovingPlan* pParentPlan)
        : MovingTask(pParentPlan)
        , m_pPlanParent(pParentPlan)
        , m_taskState(TS_Stoped)
    {
		resetCurrAction();
    }
    virtual ~MovingTask_Base(){ ; }

    virtual void taskPlay() = 0;
    virtual void taskStop() = 0;
    virtual void taskPause() = 0;

    virtual Task_State  taskState(){ return m_taskState; }

public:
    virtual void envionmentVariables_Changed_Sensor(const SensorType sensorType, const int sensorIndex, \
        const LPVOID sensorData, const int sensorDataSize) = 0;

	virtual void sensorValuesChanged(SensorType sensorType) = 0;

	virtual bool doCurrentAction(){
		Sensor sensor = ((MovingPlan_Base*)m_pPlanParent)->m_sensor;
		if (m_actionList.empty() && m_currAction.moveType == MT_NONE)											//判断动作队列是否执行完
			return false;

		if (m_currAction.moveType != MT_NONE && m_currAction.moveType != MT_Speed && !sensor.isRobotStopped())	//等待完成上一个动作
			return true;
		if (m_currAction.moveType == MT_Speed && ((sensor.mEncoder.stamp - m_lastTime) < m_currAction.time))
			return true;
		resetCurrAction();
		if (!m_actionList.empty()){
			m_currAction = m_actionList.front();
			if (m_currAction.moveType == MT_Speed){
				m_lastTime = sensor.mEncoder.stamp;
			}
			JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
			pRobot->setMoveType(m_currAction.moveType, m_currAction.param1, m_currAction.param2);
			m_actionList.erase(m_actionList.begin());
			return true;
		}
		return false;
	}

	virtual bool addAction(MoveType moveType, int param1, int param2, DWORD time=0){
		Action newAction;
		newAction.moveType = moveType;
		newAction.param1 = param1;
		newAction.param2 = param2;
		newAction.time = time;
		m_actionList.push_back(newAction);
		return true;
	}
protected:
    MovingPlan*                 m_pPlanParent;
    std::atomic<Task_State>     m_taskState;
	std::vector<Action>			m_actionList;
	Action m_currAction;
	DWORD		m_lastTime;
private:
	void resetCurrAction(){
		m_currAction.moveType = MT_NONE;
		m_currAction.param1 = 0;
		m_currAction.param2 = 0;
		m_currAction.time = 0;
	}
	void clearActionList(){
		m_actionList.clear();
		resetCurrAction();
		JoyoungRobot* pRobot = m_pPlanParent->planManager()->robot();
		pRobot->setMoveType(MT_Stop, 0, 0);
	}
};
