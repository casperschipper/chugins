#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <iostream>
#include <limits.h>
#include <math.h>

#define TABLE_SIZE 8
#define WORLD_SIZE 255


CK_DLL_CTOR(CA1_ctor);
CK_DLL_DTOR(CA1_dtor);

CK_DLL_MFUN(CA1_setRule);
CK_DLL_MFUN(CA1_getRule);

CK_DLL_TICK(CA1_tick);

t_CKINT CA1_data_offset = 0;

float clipf (float value) {
    if (value < -1) return -1.0;
    if (value > 1) return 1.0;
    return value;
}

class CA1 {
    int size;
    bool state[2][WORLD_SIZE];
    bool currentState;
    int rule;
    
    int table[TABLE_SIZE];
    
    int value;

    void calculateRuleTable() {
        int n = TABLE_SIZE;
        while (n--) {
            table[n] = rule & 1;
            rule = rule >> 1;
            fprintf(stderr, "rule number %i set to %i \n",n,table[n]);
        }
    }
    
    void init() {
        for (int i = 0;i<WORLD_SIZE;i++) {
            state[currentState][i] = (i == 64) ? 1 : 0;
        }
        currentState = 0;
    }    
public:
    CA1 () {
        size = WORLD_SIZE;
        currentState = 0;
        rule = 110;
        value = 0;
        setRule(110);
        int count;
    }

    void setRule(int ruleArg) {
        rule = ruleArg & 255; // sanity check
        calculateRuleTable();
    }

    void reseed(int seed) {
        seed = seed & size-1;
        int n = size;
        while(n--) {
            state[0][n] = seed & 1;
            seed = seed >> 1;
        }
        currentState = 0;
    }
    

    float step() 
    {
        {
            value = 0; // always start from zero
            bool nextState = !currentState; // update
            
            int a,b,c; // the neighbors

            // to avoid checks in the loop, ceparate boundary conditions.
            // first step (so the 0th cell)
            a = state[currentState][size-1]; // circular
            b = state[currentState][0];
            c = state[currentState][1];
            value += state[nextState][0] = table[(a << 2) + (b << 1) + c];
            a = b;
            b = c;

            // cells between 1 and size -2
            for (int i = 1,loopSize = size-2; i<loopSize ; i++ ) {
                c = state[currentState][i+1];
                value += state[nextState][i] = table[(a << 2) + (b << 1) + c];
                a = b;
                b = c;
            }

            // last cell
            c = state[currentState][0];
            value += state[nextState][size-1] = table[(a << 2) + (b << 1) + c];

            currentState = !currentState; // swap state buffer;

            // return the sum of values
            return (float) value / size;
        }
    }
};


struct CA1Data
{
    CA1 *ca1;
    int rule;
};


CK_DLL_QUERY(CA1)
{
    QUERY->setname(QUERY, "CA1");
    
    QUERY->begin_class(QUERY, "CA1", "UGen");
    
    QUERY->add_ctor(QUERY, CA1_ctor);
    QUERY->add_dtor(QUERY, CA1_dtor);
    
    QUERY->add_ugen_func(QUERY, CA1_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, CA1_setRule, "int", "rule");
    QUERY->add_arg(QUERY, "int", "rule");
    
    QUERY->add_mfun(QUERY, CA1_getRule, "int", "rule");
    
    CA1_data_offset = QUERY->add_mvar(QUERY, "int", "@CA1Data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(CA1_ctor)
{
    OBJ_MEMBER_INT(SELF, CA1_data_offset) = 0;
    
    CA1Data * ca1Data = new CA1Data;
    ca1Data->ca1 = new CA1();
    ca1Data->rule = 110;
    
    OBJ_MEMBER_INT(SELF, CA1_data_offset) = (t_CKINT) ca1Data;
}

CK_DLL_DTOR(CA1_dtor)
{
    CA1Data * ca1Data = (CA1Data *) OBJ_MEMBER_INT(SELF, CA1_data_offset);
    if(ca1Data)
    {
        delete ca1Data;
        OBJ_MEMBER_INT(SELF, CA1_data_offset) = 0;
        ca1Data = NULL;
    }
}

CK_DLL_TICK(CA1_tick)
{
    CA1Data * ca1Data = (CA1Data *) OBJ_MEMBER_INT(SELF, CA1_data_offset);

    float probability = 0.0;

    if (in > 0) {
        ca1Data->ca1->reseed( (int) (  floor(in / 1.0) * 2147483647 ) ) ; // please change this when the size is variable 
    }

    *out =  ca1Data->ca1->step() ;
    
    return TRUE;
}

CK_DLL_MFUN(CA1_setRule)
{
    CA1Data * ca1Data = (CA1Data *) OBJ_MEMBER_INT(SELF, CA1_data_offset);
    ca1Data->rule = GET_NEXT_INT(ARGS);
    ca1Data->ca1->setRule(ca1Data->rule);
    RETURN->v_int = ca1Data->rule;
}

CK_DLL_MFUN(CA1_getRule)
{
    CA1Data * ca1Data = (CA1Data *) OBJ_MEMBER_INT(SELF, CA1_data_offset);
    RETURN->v_int = ca1Data->rule;
}









