// EvolutionWar.cpp : Genetic Algo involving 2 AIs fighting to the death... or not?
// By Stephanie Rancourt   @StephanieRct
// Original version date: somewhen around 2009 and 2010
// instruction:
// 1-run
// 2-hit Pause (on your keyboard) to see current winner ai code*
// 3-Eat cake
// *Result may change from one run to another.
// Use, Copy, Modify, or do whatever you want want with this code. I'm not responsible for anything you do with this and it's not my fault if you die by to much cake eating. :)
// Also don't mind the ugly code, cheers!

#include <cctype>
#include <map>
#include <stack>
#include <vector>
#include <Windows.h>

#define nullptr 0
typedef unsigned int uint32;

static const uint32 gsuiNbGeneToMutate=3;
static const uint32 gsuiNbGeneToChange=2;

class Vector2D{
public:
    Vector2D(){}
    Vector2D(float a, float b)
        :x(a),y(b){}
    float getX()const{ return x; }
    float getY()const{ return y; }
    Vector2D operator+(const Vector2D& a)const{
        Vector2D v;
        v.x = x + a.x;
        v.y = y + a.y;
        return v;
    }
    Vector2D operator-(const Vector2D& a)const{
        Vector2D v;
        v.x = x - a.x;
        v.y = y - a.y;
        return v;
    }
    Vector2D operator*(const float& a)const{
        Vector2D v;
        v.x = x * a;
        v.y = y * a;
        return v;
    }
    Vector2D operator/(const float& a)const{
        Vector2D v;
        v.x = x / a;
        v.y = y / a;
        return v;
    }
    float getLength()const{
        return sqrt(x*x+y*y);
    }
    float x;
    float y;
};
typedef float Scalar;
template< class T >
T Min(const T&a, const T& b){
    return a<b?a:b;
}


class Object
{
public:
    void hit(float afHp)
    {
        mfHp -= afHp;
    }
    float mfHp;
};
class SpacialObject : public Object
{
public:
    const Vector2D& getPosition() const{ return mPosition;}
    void setPosition(const Vector2D& a){ mPosition = a;}
private:
    Vector2D mPosition;
};
class Character : public SpacialObject
{
public:
    float getSpeed(){return mfSpeed;}

    float mfSpeed;
};
class Context
{
public:
    SpacialObject* getTarget()const{return mpCurrentTarget;}
    void setTarget(SpacialObject* a){mpCurrentTarget = a;}
    Character* getFirstEnemy()const
    {
        return mpEnemy;
    }
    Character * getAiCharacter() const { return mpAiCharacter;}
    bool useActionPoint(float afPoint)
    {
        if (mfNbActionPoint >= afPoint)
        {
            mfNbActionPoint -= afPoint;
            return true;
        }
        return false;
    }
    bool hasActionPoint(float afPoint)
    {
        return (mfNbActionPoint >= afPoint);
    }
    float getActionPointCount() const { return mfNbActionPoint; }

    SpacialObject* mpCurrentTarget;
    Character* mpEnemy;
    Character * mpAiCharacter;
    float mfNbActionPoint;
};
class Condition
{
public:
    virtual bool isTrue(const Context& aCtx)const=0;
    virtual ~Condition(){}
};
class CdtHasTarget : public Condition
{
public:
    explicit CdtHasTarget(bool abHas) : mbHas(abHas){}

    virtual bool isTrue(const Context& aCtx)const
    {
        if(mbHas) 
            return aCtx.getTarget() != nullptr;
        else
            return aCtx.getTarget() == nullptr;
    }
    virtual ~CdtHasTarget(){}
    static void Randomize(CdtHasTarget& a)
    {
        a.mbHas = (rand()%2)==0;
    }
private:
    bool mbHas;
};
class Action
{
public:
    virtual void print()const=0;
    virtual void execute(Context& aCtx)=0;
    virtual ~Action(){}
    virtual Action* clone()const=0;
    virtual void mutate()=0;
};
class ChoseEnemyTarget : public Action
{
public:
    virtual void print()const{printf("ChoseEnemyTarget\n");}
    virtual void execute(Context& aCtx)
    {
        aCtx.setTarget(aCtx.getFirstEnemy());
    }
    virtual ~ChoseEnemyTarget(){}

    virtual Action* clone()const{ return new ChoseEnemyTarget(); }
    virtual void mutate(){}
};
class MoveToTargetRange : public Action
{
public:
    virtual ~MoveToTargetRange(){}

    virtual void print()const{printf("MoveToTargetRange range=%f\n", mRange);}
    virtual void execute(Context& aCtx)
    {
        if ( aCtx.getTarget() )
        {
            Vector2D vDest = aCtx.getTarget()->getPosition();
            Vector2D vDirection = vDest - aCtx.getAiCharacter()->getPosition();
            Scalar length = vDirection.getLength();
            Scalar distLeft = length - mRange;
            Scalar distToDo = Min(distLeft, aCtx.getAiCharacter()->getSpeed() * aCtx.getActionPointCount() );
            float fActionPoint = distToDo / aCtx.getAiCharacter()->getSpeed();
            Vector2D vDelta = vDirection * (distToDo / length);
            aCtx.getAiCharacter()->setPosition( aCtx.getAiCharacter()->getPosition() + vDelta );
            aCtx.useActionPoint(fActionPoint);

        }
    }
    static void Randomize(MoveToTargetRange& a)
    {
        a.mRange = (rand()%1000) / 1000.0f * 10;
    }
    virtual Action* clone()const
    { 
        return new MoveToTargetRange(*this); 
    }
    virtual void mutate()
    {
        mRange += ((rand()%1000) / 1000.0f * 1)-0.5f;
    }
    float mRange;
};

class MeeleAttackTarget : public Action
{
public:

    virtual ~MeeleAttackTarget(){}
    virtual void print()const{printf("MeeleAttackTarget\n");}
    virtual void execute(Context& aCtx)
    {
        if ( aCtx.getTarget() )
        {
            Scalar range = (aCtx.getTarget()->getPosition() - aCtx.getAiCharacter()->getPosition()).getLength();
            if ( range < 1.0f && aCtx.hasActionPoint(1) )
            {
                aCtx.useActionPoint(1);
                aCtx.getTarget()->hit(1);
            }
        }
    }
    virtual Action* clone()const{ return new MeeleAttackTarget(); }
    virtual void mutate(){}
};
class RangeAttackTarget : public Action
{
public:

    virtual ~RangeAttackTarget(){}
    virtual void print()const{printf("RangeAttackTarget\n");}
    virtual void execute(Context& aCtx)
    {
        if ( aCtx.getTarget() )
        {
            Scalar range = (aCtx.getTarget()->getPosition() - aCtx.getAiCharacter()->getPosition()).getLength();
            if ( range < 8.0f && aCtx.hasActionPoint(1) )
            {
                aCtx.useActionPoint(1);
                aCtx.getTarget()->hit(0.1);
            }
        }
    }
    virtual Action* clone()const{ return new RangeAttackTarget(); }
    virtual void mutate(){}
};
class EatCake : public Action
{
public:

    virtual ~EatCake(){}
    virtual void print()const{printf("EatCake\n");}
    virtual void execute(Context& aCtx)
    {
        if( aCtx.hasActionPoint(2) )
        {
            aCtx.getAiCharacter()->mfHp += 0.1;
            aCtx.useActionPoint(2);
        }
    }
    virtual Action* clone()const{ return new EatCake(); }
    virtual void mutate(){}
};
class SkipTurn : public Action
{
public:

    virtual ~SkipTurn(){}
    virtual void print()const{printf("SkipTurn\n");}
    virtual void execute(Context& aCtx)
    {
        aCtx.useActionPoint(aCtx.getActionPointCount());
    }
    virtual Action* clone()const{ return new SkipTurn(); }
    virtual void mutate(){}
};
class Ai
{
public:
    struct GcAtom
    {
        GcAtom()
            :mpAction(nullptr){}
        GcAtom(const GcAtom& a)
            :mpAction(nullptr){}
        GcAtom& operator=(const GcAtom& a)
        {
            mpAction=nullptr;
            return *this;
        }
        Action * mpAction;

    };
    explicit Ai(uint32 auiCodeSize)
        : mGeneticCode(auiCodeSize)
    {

    }
    ~Ai()
    {
        for(uint32 i = 0; i < mGeneticCode.size() ; ++i)
        {
            delete mGeneticCode[i].mpAction;
        }
        mGeneticCode.clear();
    }
    void execute(Context& aCtx)
    {
        for(uint32 i = 0; i < mGeneticCode.size() && aCtx.getActionPointCount() > 0 ; ++i)
        {
            mGeneticCode[i].mpAction->execute(aCtx);
        }
    }
    void printCode()const
    {
        for(uint32 i = 0; i < mGeneticCode.size() ; ++i)
        {
            mGeneticCode[i].mpAction->print();
        }

    }
    void cloneTo(Ai* aTo)const
    {
        aTo->mGeneticCode.resize(mGeneticCode.size());

        for(uint32 i = 0; i < mGeneticCode.size() ; ++i)
        {
            aTo->mGeneticCode[i].mpAction = mGeneticCode[i].mpAction->clone();
        }
    }
    std::vector<GcAtom> mGeneticCode;

private:
    Ai(const Ai& a)
    {

    }
    Ai& operator=(const Ai&a)
    {
        return *this;
    }
};
Action* NewRandomAction()
{
    uint32 uiRand = rand()%6;
    switch (uiRand)
    {
    case 0:
        return new ChoseEnemyTarget();
    case 1:
        {
            MoveToTargetRange * pAction = new MoveToTargetRange();
            MoveToTargetRange::Randomize(*pAction);
            return pAction;
        }
    case 2:
        return new MeeleAttackTarget();
    case 3:
        return new RangeAttackTarget();
    case 4:
        return new SkipTurn();
    case 5:
        return new EatCake();
    }
}
static const uint32 gsuiNbGene = 12;
Ai * NewRandomAi()
{
    Ai * pAi = new Ai(gsuiNbGene);
    for(uint32 i = 0 ; i < gsuiNbGene; ++i)
    {
        pAi->mGeneticCode[i].mpAction = NewRandomAction();
    }
    return pAi;
}
Ai * NewMutatedAi(Ai* apAi)
{
    Ai * pAi = new Ai(gsuiNbGene);
    apAi->cloneTo(pAi);
    //mutate genes
    for(uint32 i = 0 ; i < gsuiNbGeneToMutate; ++i)
    {
        uint32 index= rand()%gsuiNbGene;
        pAi->mGeneticCode[index].mpAction->mutate();
    }
    //create new genes
    for(uint32 i = 0 ; i < gsuiNbGeneToChange; ++i)
    {
        uint32 index= rand()%gsuiNbGene;
        delete pAi->mGeneticCode[index].mpAction;
        pAi->mGeneticCode[index].mpAction = NewRandomAction();
    }
    return pAi;
}
int _tmain(int argc, _TCHAR* argv[])
{
    uint32 uiSeed = ::GetTickCount();
    srand(uiSeed);
    uint32 uiNbGeneration=0;
    uint32 uiLastWinner = -1;
    Character ch0;

    Character ch1;

    ch0.mfSpeed = 1;
    ch1.mfSpeed = 1;

    ch0.setPosition(Vector2D(0,0));
    ch1.setPosition(Vector2D(10,0));

    Ai* pAi0 = NewRandomAi();
    Ai* pAi1 = NewRandomAi();

    ch0.mfHp=10;
    Context ctx0;
    ctx0.mpAiCharacter = &ch0;
    ctx0.mpCurrentTarget = 0;
    ctx0.mpEnemy = &ch1;
    ctx0.mfNbActionPoint = 0;

    ch1.mfHp=10;
    Context ctx1;
    ctx1.mpAiCharacter = &ch1;
    ctx1.mpCurrentTarget = 0;
    ctx1.mpEnemy = &ch0;
    ctx1.mfNbActionPoint = 0;
    static const uint32 suiMaxIteration = 500;
    uint32 uiScore0=0;
    uint32 uiScore1=0;
    uint32 uiScoreNull=0;
    HWND consoleWindow = GetConsoleWindow();
    while(1)
    {
        printf("===== Generation %d ===== \n", uiNbGeneration);
        uint32 uiRoundCount=0;
        while(uiRoundCount < suiMaxIteration)
        {
            ctx0.mfNbActionPoint += 5;
            ctx1.mfNbActionPoint += 5;
            uint32 uiOrder = rand()%2;
            switch(uiOrder){
            case 0:
                pAi0->execute(ctx0);
                pAi1->execute(ctx1);
                break;
            case 1:
                pAi1->execute(ctx1);
                pAi0->execute(ctx0);
                break;
            }


            if ( ch0.mfHp < 0 || ch1.mfHp < 0 ){
                break;
            }

            ++uiRoundCount;
        }
        if ( ch0.mfHp <= 0 )
        {
            uiLastWinner = 1;
            printf("Ai 0 die. Mutate 1 to 0\n");
            printf("Ai 1 (winner) code:\n");
            pAi1->printCode();
            delete pAi0;
            //pAi0 = NewRandomAi();
            pAi0 = NewMutatedAi(pAi1);
            ++uiScore1;
        }
        if ( ch1.mfHp <= 0 )
        {
            uiLastWinner = 0;
            printf("Ai 1 die. Mutate 0 to 1\n");
            printf("Ai 0 (winner) code:\n");
            pAi0->printCode();
            delete pAi1;
            //pAi1 = NewRandomAi();
            pAi1 = NewMutatedAi(pAi0);
            ++uiScore0;
        }
        if ( uiRoundCount>=suiMaxIteration && ch0.mfHp > 0 && ch1.mfHp > 0 )
        {
            ++uiScoreNull;
            if ( ch0.mfHp > ch1.mfHp ){

                uiLastWinner = 0;
                printf("No dead. HP 0 > 1. Mutate 0 to 1\n");
                printf("Ai 0 (winner) code:\n");
                pAi0->printCode();
                delete pAi1;
                pAi1 = NewMutatedAi(pAi0);
            }else if ( ch1.mfHp > ch0.mfHp ){
                uiLastWinner = 1;
                printf("No dead. HP 1 > 0. Mutate 1 to 0\n");
                printf("Ai 1 (winner) code:\n");
                pAi1->printCode();
                delete pAi0;
                pAi0 = NewMutatedAi(pAi1);
            } else if(ch0.mfHp == ch1.mfHp){
                if(uiLastWinner==0){
                    printf("No winner. Last winner was 0. Mutate 0 to 1\n");
                    printf("Ai 0 (winner) code:\n");
                    pAi0->printCode();
                    delete pAi1;
                    pAi1 = NewMutatedAi(pAi0);
                } else if(uiLastWinner==1){
                    printf("No winner. Last winner was 1. Mutate 1 to 0\n");
                    printf("Ai 1 code:\n");
                    pAi0->printCode();
                    delete pAi0;
                    pAi0 = NewMutatedAi(pAi1);
                } else {
                    printf("No winner. No last winner. Create new AIs\n");
                    delete pAi0;
                    delete pAi1;
                    pAi0 = NewRandomAi();
                    pAi1 = NewRandomAi();
                }
            }
        }
        char buff[256];
        sprintf_s(buff,256,"Score: 0: %8d    1: %8d   Null: %8d", uiScore0, uiScore1, uiScoreNull);
        ::SetWindowTextA(consoleWindow , buff);

        ctx0.mpAiCharacter = &ch0;
        ctx0.mpCurrentTarget = 0;
        ctx0.mpEnemy = &ch1;
        ctx0.mfNbActionPoint = 0;
        ctx1.mpAiCharacter = &ch1;
        ctx1.mpCurrentTarget = 0;
        ctx1.mpEnemy = &ch0;
        ctx1.mfNbActionPoint = 0;
        ch0.setPosition(Vector2D(0,0));
        ch1.setPosition(Vector2D(10,0));
        ch0.mfHp=10;
        ch1.mfHp=10;
        ++uiNbGeneration;
    }

}
