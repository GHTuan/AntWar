#include "Physic2D.hpp"
#include "Components.hpp"

class AIController : public Component {
    protected:
        Rigidbody2D *rigidbody;
        MovementController *movementController;
    
        GameObject *target;
    
        float speed = 0;
    
        float alertZoneXStart = 0, alertZoneXEnd = 0;
        float dangerZoneXStart = 0, dangerZoneXEnd = 0;
    
        bool isTeam1 = false;
    
    public:
        AIController(GameObject *parent, GameObject *target, float speed, bool isTeam1) : Component(parent) {
            this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
            this->movementController = this->gameObject->GetComponent<MovementController>();
            this->target = target;
            this->speed = speed;
    
            this->isTeam1 = isTeam1;
        }
    
        virtual void Update() = 0;
    
        void Draw() {}
    
        virtual Component *Clone(GameObject *parent) = 0;
    };
    
    class AIGoalKeeper : public AIController {
    private:
        float dangerZoneYStart = 0, dangerZoneYEnd = 0;
    
    public:
        AIGoalKeeper(GameObject *parent, GameObject *target, float speed, bool isTeam1) : AIController(parent, target, speed, isTeam1) {
            if (isTeam1) {
                dangerZoneXStart = 0;
                dangerZoneXEnd = 20.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 20.0f / 100.0f * WIDTH;
                alertZoneXEnd = 60.0f / 100.0f * WIDTH;
    
            } else {
                dangerZoneXStart = 80.0f / 100.0f * WIDTH;
                dangerZoneXEnd = 100.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 40.0f / 100.0f * WIDTH;
                alertZoneXEnd = 80.0f / 100.0f * WIDTH;
            }
    
            dangerZoneYStart = 0.0f / 100.0f * HEIGHT;
            dangerZoneYEnd = 100.0f / 100.0f * HEIGHT;
    
            this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
        }
    
        void Update() {
            if (rigidbody == nullptr)
                return;
            if (movementController != nullptr && movementController->GetEnabled())
                return;
    
            float actualSpeed = speed * 1 / FPS;
    
            Vector2 targetPosition = target->transform.position;
            Rigidbody2D *targetRigidbody = target->GetComponent<Rigidbody2D>();
    
            Vector2 currentPosition = gameObject->transform.position;
    
            GameObject *ballBindedBy = target->GetComponent<BallStateMachine>()->GetBinded();
            bool teamHasBall = ballBindedBy != nullptr && ballBindedBy->tag == gameObject->tag;
    
            // Binded to ball
            if (ballBindedBy != nullptr && ballBindedBy == gameObject) {
                target->GetComponent<BallStateMachine>()->Kick(Vector2(isTeam1 ? 1 : -1, 0), HIGH_KICK_FORCE, gameObject);
            } else
                // Target is in the alert zone
                if (targetPosition.x >= alertZoneXStart && targetPosition.x <= alertZoneXEnd && !teamHasBall) {
    
                    if (targetPosition.y < currentPosition.y) {
                        rigidbody->AddForce(Vector2(0, -1).Normalize() * actualSpeed);
                    } else if (targetPosition.y > currentPosition.y) {
                        rigidbody->AddForce(Vector2(0, 1).Normalize() * actualSpeed);
                    }
                }
    
                // Target is in the danger zone
                else if (targetPosition.x >= dangerZoneXStart && targetPosition.x <= dangerZoneXEnd &&
                         targetPosition.y >= dangerZoneYStart && targetPosition.y <= dangerZoneYEnd && !teamHasBall) {
                    Vector2 direction = (targetPosition - currentPosition).Normalize();
                    // Prioritize running toward target position y
                    rigidbody->AddForce(Vector2(direction.x / 4, direction.y * 4).Normalize() * actualSpeed);
                }
    
                // Target is neither, restore original position, or team has control of ball
                else {
                    Vector2 dangerZoneCenter((dangerZoneXStart + dangerZoneXEnd) / 2, (dangerZoneYStart + dangerZoneYEnd) / 2);
                    Vector2 direction = (dangerZoneCenter - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
        }
    
        Component *Clone(GameObject *parent) {
            AIGoalKeeper *newAIGoalKeeper = new AIGoalKeeper(parent, target, speed, isTeam1);
            return newAIGoalKeeper;
        }
    };
    
    class AIDefender : public AIController {
    public:
        AIDefender(GameObject *parent, GameObject *target, float speed, bool isTeam1) : AIController(parent, target, speed, isTeam1) {
            if (isTeam1) {
                dangerZoneXStart = 0.0f / 100.0f * WIDTH;
                dangerZoneXEnd = 50.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 50.0f / 100.0f * WIDTH;
                alertZoneXEnd = 75.0f / 100.0f * WIDTH;
            } else {
                dangerZoneXStart = 50.0f / 100.0f * WIDTH;
                dangerZoneXEnd = 100.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 25.0f / 100.0f * WIDTH;
                alertZoneXEnd = 50.0f / 100.0f * WIDTH;
            }
    
            this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
        }
    
        void Update() {
            if (rigidbody == nullptr)
                return;
            if (movementController != nullptr && movementController->GetEnabled())
                return;
    
            float actualSpeed = speed * 1 / FPS;
    
            Vector2 targetPosition = target->transform.position;
            Vector2 currentPosition = gameObject->transform.position;
    
            GameObject *ballBindedBy = target->GetComponent<BallStateMachine>()->GetBinded();
            bool teamHasBall = ballBindedBy != nullptr && ballBindedBy->tag == gameObject->tag;
    
            // Binded to ball
            if (ballBindedBy != nullptr && ballBindedBy == gameObject) {
                target->GetComponent<BallStateMachine>()->Kick(Vector2(isTeam1 ? 1 : -1, 0), LOW_KICK_FORCE, gameObject);
            } else
    
                // Target is in the alert zone
                if (targetPosition.x >= alertZoneXStart && targetPosition.x <= alertZoneXEnd) {
                    Vector2 direction = (targetPosition - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
    
                // Target is in the danger zone
                else if (targetPosition.x >= dangerZoneXStart && targetPosition.x <= dangerZoneXEnd) {
                    Vector2 direction = (targetPosition - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
    
                // Target is neither, restore original position
                else {
                    Vector2 dangerZoneCenter((dangerZoneXStart + dangerZoneXEnd) / 2, WIDTH / 2);
                    Vector2 direction = (dangerZoneCenter - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
        }
    
        Component *Clone(GameObject *parent) {
            AIDefender *newAIDefender = new AIDefender(parent, target, speed, isTeam1);
            return newAIDefender;
        }
    };
    
    class AIAttacker : public AIController {
    public:
        AIAttacker(GameObject *parent, GameObject *target, float speed, bool isTeam1) : AIController(parent, target, speed, isTeam1) {
            if (isTeam1) {
                dangerZoneXStart = 50.0f / 100.0f * WIDTH;
                dangerZoneXEnd = 100.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 100.0f / 100.0f * WIDTH;
                alertZoneXEnd = 100.0f / 100.0f * WIDTH;
            } else {
                dangerZoneXStart = 0.0f;
                dangerZoneXEnd = 50.0f / 100.0f * WIDTH;
    
                alertZoneXStart = 0.0f / 100.0f * WIDTH;
                alertZoneXEnd = 0.0f / 100.0f * WIDTH;
            }
    
            rigidbody = gameObject->GetComponent<Rigidbody2D>();
        }
    
        void Update() {
            if (rigidbody == nullptr)
                return;
            if (movementController != nullptr && movementController->GetEnabled())
                return;
    
            float actualSpeed = speed * 1 / FPS;
    
            Vector2 targetPosition = target->transform.position;
            Vector2 currentPosition = gameObject->transform.position;
    
            GameObject *ballBindedBy = target->GetComponent<BallStateMachine>()->GetBinded();
            bool teamHasBall = ballBindedBy != nullptr && ballBindedBy->tag == gameObject->tag;
    
            // Binded to ball
            if (ballBindedBy != nullptr && ballBindedBy == gameObject) {
                Vector2 goalPosition = isTeam1 ? Vector2(95.0f / 100.0f * WIDTH, HEIGHT / 2) : Vector2(5.0f / 100.0f * WIDTH, HEIGHT / 2);
                Vector2 direction = (goalPosition - currentPosition).Normalize();
    
                // In optimal y position to goal
                bool inOptimalYPosition = (20.0 / 100 * HEIGHT <= currentPosition.y && currentPosition.y <= 80.0 / 100 * HEIGHT);
    
                // Near goal conditions
                bool nearGoalTeam1 = (isTeam1 && currentPosition.x >= 75.0f / 100.0f * WIDTH && currentPosition.x <= 85.0f / 100.0f * WIDTH);
                bool nearGoalTeam2 = (!isTeam1 && currentPosition.x <= 25.0f / 100.0f * WIDTH && currentPosition.x >= 15.0f / 100.0f * WIDTH);
    
                // Check if AI is behind the goal
                bool behindGoalTeam1 = (isTeam1 && currentPosition.x > 92.0f / 100.0f * WIDTH);
                bool behindGoalTeam2 = (!isTeam1 && currentPosition.x < 8.0f / 100.0f * WIDTH);
    
                if (inOptimalYPosition && (nearGoalTeam1 || nearGoalTeam2)) {
                    target->GetComponent<BallStateMachine>()->Kick(direction, HIGH_KICK_FORCE, gameObject);
                    return;
                }
    
                // If AI is behind the goal, move it toward the front of the goal
                if (behindGoalTeam1 || behindGoalTeam2) {
                    Vector2 frontOfGoal = isTeam1 ? Vector2(85.0f / 100.0f * WIDTH, HEIGHT / 2) : Vector2(15.0f / 100.0f * WIDTH, HEIGHT / 2);
                    direction = (frontOfGoal - currentPosition).Normalize();
                }
    
                // Not near goal, run toward goal
                rigidbody->AddForce(direction * actualSpeed);
            } else
    
                // Target is in the alert zone
                if (targetPosition.x >= alertZoneXStart && targetPosition.x <= alertZoneXEnd) {
                    Vector2 direction = (targetPosition - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
    
                // Target is in the danger zone
                else if (targetPosition.x >= dangerZoneXStart && targetPosition.x <= dangerZoneXEnd) {
                    Vector2 direction = (targetPosition - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
    
                // Target is neither, restore original position
                else {
                    Vector2 dangerZoneCenter((dangerZoneXStart + dangerZoneXEnd) / 2, currentPosition.y);
                    Vector2 direction = (dangerZoneCenter - currentPosition).Normalize();
                    rigidbody->AddForce(direction * actualSpeed);
                }
        }
    
        Component *Clone(GameObject *parent) {
            AIAttacker *newAIAttacker = new AIAttacker(parent, target, speed, isTeam1);
            return newAIAttacker;
        }
    };