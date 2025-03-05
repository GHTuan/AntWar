#ifndef AICONTROLLER_HPP
#define AICONTROLLER_HPP

#include "Physic2D.hpp"
#include "Components.hpp"

class AIController : public Component
{
protected:
    Rigidbody2D *rigidbody;
    MovementController *movementController;

    GameObject *target;

    float speed = 0;

    float alertZoneXStart = 0, alertZoneXEnd = 0;
    float dangerZoneXStart = 0, dangerZoneXEnd = 0;

    bool isTeam1 = false;

public:
    AIController(GameObject *parent, GameObject *target, float speed, bool isTeam1) : Component(parent)
    {
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

class AIGoalKeeper : public AIController
{
private:
    float dangerZoneYStart = 0, dangerZoneYEnd = 0;

public:
    AIGoalKeeper(GameObject *parent, GameObject *target, float speed, bool isTeam1) : AIController(parent, target, speed, isTeam1)
    {
        if (isTeam1)
        {
            dangerZoneXStart = 12.0f / 100.0f * WIDTH;
            dangerZoneXEnd = 20.0f / 100.0f * WIDTH;

            alertZoneXStart = 20.0f / 100.0f * WIDTH;
            alertZoneXEnd = 60.0f / 100.0f * WIDTH;
        }
        else
        {
            dangerZoneXStart = 80.0f / 100.0f * WIDTH;
            dangerZoneXEnd = 88.0f / 100.0f * WIDTH;

            alertZoneXStart = 40.0f / 100.0f * WIDTH;
            alertZoneXEnd = 80.0f / 100.0f * WIDTH;
        }

        dangerZoneYStart = 30.0f / 100.0f * HEIGHT;
        dangerZoneYEnd = 70.0f / 100.0f * HEIGHT;

        this->rigidbody = this->gameObject->GetComponent<Rigidbody2D>();
    }

    void Update()
    {
        if (rigidbody == nullptr)
            return;
        if (movementController != nullptr && movementController->GetEnabled())
            return;
    
        float actualSpeed = speed * 1 / FPS;
        Vector2 targetPosition = target->transform.position;
        Vector2 currentPosition = gameObject->transform.position;
    
        GameObject *ballBindedBy = target->GetComponent<BallStateMachine>()->GetBinded();
        bool teamHasBall = ballBindedBy != nullptr && ballBindedBy->tag == gameObject->tag;
    
        // Nếu thủ môn đang giữ bóng, chuyền lên
        if (ballBindedBy != nullptr && ballBindedBy == gameObject)
        {
            target->GetComponent<BallStateMachine>()->Kick(Vector2(isTeam1 ? 1 : -1, 0), HIGH_KICK_FORCE, gameObject);
            return;
        }
    
        // Nếu bóng vào vùng cảnh báo, di chuyển theo trục Y
        if (targetPosition.x >= alertZoneXStart && targetPosition.x <= alertZoneXEnd && !teamHasBall)
        {
            if (targetPosition.y < currentPosition.y)
            {
                rigidbody->AddForce(Vector2(0, -1).Normalize() * actualSpeed);
            }
            else if (targetPosition.y > currentPosition.y)
            {
                rigidbody->AddForce(Vector2(0, 1).Normalize() * actualSpeed);
            }
        }
        // Nếu bóng vào vùng nguy hiểm, di chuyển nhanh hơn
        else if (targetPosition.x >= dangerZoneXStart && targetPosition.x <= dangerZoneXEnd &&
                 targetPosition.y >= dangerZoneYStart && targetPosition.y <= dangerZoneYEnd && !teamHasBall)
        {
            Vector2 direction = (targetPosition - currentPosition).Normalize();
            rigidbody->AddForce(Vector2(direction.x / 4, direction.y * 4).Normalize() * actualSpeed);
        }
        // Nếu bóng không nguy hiểm, trở về vị trí trung tâm vùng an toàn
        else
        {
            Vector2 dangerZoneCenter((dangerZoneXStart + dangerZoneXEnd) / 2, (dangerZoneYStart + dangerZoneYEnd) / 2);
            Vector2 direction = (dangerZoneCenter - currentPosition).Normalize();
            rigidbody->AddForce(direction * actualSpeed);
        }
    
        // 🌟 **Giới hạn vùng di chuyển của thủ môn (mượt hơn)**
        float smoothFactor = 0.1f; // Hệ số làm mượt (càng nhỏ càng mượt)
        if (gameObject->transform.position.y < dangerZoneYStart)
        {
            gameObject->transform.position.y = (1 - smoothFactor) * gameObject->transform.position.y + smoothFactor * dangerZoneYStart;
        }
        if (gameObject->transform.position.y > dangerZoneYEnd)
        {
            gameObject->transform.position.y = (1 - smoothFactor) * gameObject->transform.position.y + smoothFactor * dangerZoneYEnd;
        }
    
        // 🌟 **Thêm lực cản nhẹ**
        rigidbody->velocity.operator*(0.95); // Giảm tốc từ từ để tránh giật
    }
    
    

    Component *Clone(GameObject *parent)
    {
        AIGoalKeeper *newAIGoalKeeper = new AIGoalKeeper(parent, target, speed, isTeam1);
        return newAIGoalKeeper;
    }
};

class AIDefender : public AIController
{
private:
    bool isLeftDefender;          // Hậu vệ trái/phải
    GameObject *targetStriker;    // Tiền đạo đối phương để kèm
    GameObject *teammateAttacker; // Attacker cùng team để chuyền bóng
    Vector2 homePosition;         // Vị trí phòng thủ mặc định
    float reactDistance;          // Khoảng cách tối thiểu để áp sát bóng
    float markingDistance;        // Khoảng cách để kèm tiền đạo đối phương
    float passDistance;           // Khoảng cách tối thiểu để chuyền bóng
    bool hasBall;                 // Kiểm tra nếu hậu vệ đang giữ bóng

public:
    AIDefender(GameObject *parent, GameObject *target, GameObject *striker, GameObject *attacker, float speed, bool isTeam1, bool isLeft)
        : AIController(parent, target, speed, isTeam1), isLeftDefender(isLeft), targetStriker(striker), teammateAttacker(attacker)
    {

        // Xác định vị trí phòng thủ mặc định
        float defenseX = isTeam1 ? WIDTH * 0.3f : WIDTH * 0.7f;
        float defenseY = isLeftDefender ? HEIGHT * 0.35f : HEIGHT * 0.65f;
        homePosition = Vector2(defenseX, defenseY);

        // Khu vực phòng thủ
        if (isTeam1)
        {
            dangerZoneXStart = 0.0f;
            dangerZoneXEnd = WIDTH * 0.45f;
            alertZoneXStart = WIDTH * 0.45f;
            alertZoneXEnd = WIDTH * 0.65f;
        }
        else
        {
            dangerZoneXStart = WIDTH * 0.55f;
            dangerZoneXEnd = WIDTH;
            alertZoneXStart = WIDTH * 0.35f;
            alertZoneXEnd = WIDTH * 0.55f;
        }

        reactDistance = WIDTH * 0.18f;   // Tăng độ nhạy khi bóng gần vùng phòng thủ
        markingDistance = WIDTH * 0.12f; // Khoảng cách để bám theo tiền đạo
        passDistance = WIDTH * 0.2f;     // Khoảng cách để chuyền bóng cho attacker
        hasBall = false;

        rigidbody = gameObject->GetComponent<Rigidbody2D>();
    }

    void Update() override
    {
        if (!rigidbody)
            return;
        if (movementController && movementController->GetEnabled())
            return;

        float actualSpeed = speed * (1.0f / FPS);
        Vector2 targetPos = target->transform.position;
        Vector2 strikerPos = targetStriker->transform.position;
        Vector2 attackerPos = teammateAttacker->transform.position;
        Vector2 currentPos = gameObject->transform.position;

        GameObject *ballOwner = target->GetComponent<BallStateMachine>()->GetBinded();
        bool teamHasBall = (ballOwner && ballOwner->tag == gameObject->tag);
        hasBall = (ballOwner == gameObject);

        // **1. Nếu hậu vệ đang giữ bóng → Chuyền lên cho attacker**
        if (hasBall)
        {
            Vector2 passDirection = (attackerPos - currentPos).Normalize();
            target->GetComponent<BallStateMachine>()->Kick(passDirection, LOW_KICK_FORCE, gameObject);
            return;
        }

        // **2. Nếu đội nhà có bóng → Giữ cự ly phòng thủ hợp lý**
        if (teamHasBall)
        {
            MoveToPosition(homePosition, actualSpeed * 0.8f);
            return;
        }

        // **3. Nếu tiền đạo đối phương ở gần, bám sát để cản phá**
        float distanceToStriker = (strikerPos - currentPos).Magnitude();
        if (distanceToStriker < markingDistance)
        {
            MoveToPosition(strikerPos, actualSpeed);
        }
        // **4. Nếu bóng vào vùng nguy hiểm, áp sát bóng**
        else if ((targetPos - currentPos).Magnitude() < reactDistance)
        {
            MoveToPosition(targetPos, actualSpeed);
        }
        // **5. Nếu không có tình huống nguy hiểm, giữ vị trí phòng thủ**
        else
        {
            MoveToPosition(homePosition, actualSpeed * 0.8f);
        }
    }

    void MoveToPosition(Vector2 targetPos, float speed)
    {
        Vector2 direction = (targetPos - gameObject->transform.position).Normalize();
        rigidbody->AddForce(direction * speed);
    }

    Component *Clone(GameObject *parent) override
    {
        return new AIDefender(parent, target, targetStriker, teammateAttacker, speed, isTeam1, isLeftDefender);
    }
};

class AIAttacker : public AIController
{
private:
    GameObject *teammate;
    Vector2 homePosition;
    float passDistance;
    float shootDistance;
    bool hasBall;
    bool isLeftAttacker;

public:
    AIAttacker(GameObject *parent, GameObject *target, GameObject *teammate, float speed, bool isTeam1, bool isLeft)
        : AIController(parent, target, speed, isTeam1), teammate(teammate), isLeftAttacker(isLeft)
    {
        float attackX = isTeam1 ? WIDTH * 0.65f : WIDTH * 0.35f;
        float attackY = isLeftAttacker ? HEIGHT * 0.3f : HEIGHT * 0.7f;
        homePosition = Vector2(attackX, attackY);

        passDistance = WIDTH * 0.25f;
        shootDistance = WIDTH * 0.15f;
        hasBall = false;
        rigidbody = gameObject->GetComponent<Rigidbody2D>();
    }

    void Update() override
    {
        if (!rigidbody) return;
        if (movementController && movementController->GetEnabled()) return;

        float actualSpeed = speed * (1.0f / FPS);
        Vector2 targetPos = target->transform.position;
        Vector2 currentPos = gameObject->transform.position;
        Vector2 goalPos = isTeam1 ? Vector2(WIDTH * 0.95f, HEIGHT / 2)
                                  : Vector2(WIDTH * 0.05f, HEIGHT / 2);

        GameObject *ballOwner = target->GetComponent<BallStateMachine>()->GetBinded();
        bool teamHasBall = (ballOwner && ballOwner->tag == gameObject->tag);
        hasBall = (ballOwner == gameObject);

        if (hasBall)
        {
            if (ShouldPass(currentPos, teammate->transform.position))
            {
                Pass(teammate->transform.position);
            }
            else if (ShouldDribble(currentPos, goalPos))
            {
                Dribble(goalPos, actualSpeed);
            }
            else
            {
                Shoot(goalPos);
            }
            return;
        }

        if (teamHasBall)
        {
            MaintainPosition(actualSpeed);
        }
        else if (IsBallInAttackerZone(targetPos))
        {
            MoveToPosition(targetPos, actualSpeed);
        }
        else
        {
            MoveToRestPosition(actualSpeed);
        }
    }

    bool ShouldPass(Vector2 attackerPos, Vector2 teammatePos)
    {
        if ((attackerPos - teammatePos).Magnitude() > passDistance) return false;
        if (isTeam1 && attackerPos.x > teammatePos.x) return false;
        if (!isTeam1 && attackerPos.x < teammatePos.x) return false;
        return !IsDefenderBlockingPath(attackerPos, teammatePos);
    }

    bool ShouldDribble(Vector2 attackerPos, Vector2 goalPos)
    {
        return (attackerPos - goalPos).Magnitude() > shootDistance;
    }

    void Dribble(Vector2 goalPos, float speed)
    {
        Vector2 direction = (goalPos - gameObject->transform.position).Normalize();
        rigidbody->AddForce(direction * speed);
    }

    void Pass(Vector2 teammatePos)
    {
        Vector2 passDirection = (teammatePos - gameObject->transform.position).Normalize();
        target->GetComponent<BallStateMachine>()->Kick(passDirection, LOW_KICK_FORCE, gameObject);
    }

    void Shoot(Vector2 goalPos)
    {
        Vector2 shootDirection = (goalPos - gameObject->transform.position).Normalize();
        target->GetComponent<BallStateMachine>()->Kick(shootDirection, HIGH_KICK_FORCE, gameObject);
    }

    void MaintainPosition(float speed)
    {
        Vector2 strategicPosition = homePosition;
        MoveToPosition(strategicPosition, speed * 0.8f);
    }

    void MoveToRestPosition(float speed)
    {
        Vector2 restPosition = homePosition;
        restPosition.x = isTeam1 ? WIDTH * 0.65f : WIDTH * 0.35f;
        restPosition.y = isLeftAttacker ? HEIGHT * 0.35f : HEIGHT * 0.65f;
        MoveToPosition(restPosition, speed * 0.5f);
    }

    bool IsBallInAttackerZone(Vector2 ballPos)
    {
        float minX = isTeam1 ? WIDTH * 0.35f : WIDTH * 0.10f;
        float maxX = isTeam1 ? WIDTH * 0.90f : WIDTH * 0.65f;
        return (ballPos.x >= minX && ballPos.x <= maxX);
    }

    void MoveToPosition(Vector2 targetPos, float speed)
    {
        if (isLeftAttacker)
        {
            targetPos.y = std::min(targetPos.y, HEIGHT * 0.5f);
        }
        else
        {
            targetPos.y = std::max(targetPos.y, HEIGHT * 0.5f);
        }

        if (isTeam1)
        {
            targetPos.x = std::clamp(targetPos.x, WIDTH * 0.35f, WIDTH * 0.9f);
        }
        else
        {
            targetPos.x = std::clamp(targetPos.x, WIDTH * 0.1f, WIDTH * 0.65f);
        }

        Vector2 direction = (targetPos - gameObject->transform.position).Normalize();
        rigidbody->AddForce(direction * speed);
    }

    bool IsDefenderBlockingPath(Vector2 start, Vector2 end)
    {
        std::vector<GameObject *> defenders = GameObjectManager::GetInstance()->GetGameObjectsByTag(isTeam1 ? 2 : 1);
        for (GameObject *defender : defenders)
        {
            Vector2 defenderPos = defender->transform.position;
            float distance = (end - start).Magnitude();
            float projection = ((defenderPos - start).Dot(end - start)) / distance;
            if (projection > 0 && projection < distance)
            {
                Vector2 closestPoint = start + (end - start).Normalize() * projection;
                if ((defenderPos - closestPoint).Magnitude() < WIDTH * 0.05f)
                    return true;
            }
        }
        return false;
    }

    Component *Clone(GameObject *parent) override
    {
        return new AIAttacker(parent, target, teammate, speed, isTeam1, isLeftAttacker);
    }
};





#endif