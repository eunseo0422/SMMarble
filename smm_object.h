#ifndef smm_object_h
#define smm_object_h

#define SMMNODE_TYPE_MAX                7

typedef enum smmObjType {
    smmObjType_board = 0,
    smmObjType_food,
    smmObjType_festival,
    smmObjType_grade
} smmObjType_e;

/* node type :
    lecture,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
*/
typedef enum smmObjNode {     
    lecture = 0,
    restaurant,
    laboratory,
    home,
    experiment,
    foodChance,
    festival
} smmObjNode_e;

/* grade :
    A+,
    A0,
    A-,
    B+,
    B0,
    B-,
    C+,
    C0,
    C-
*/
typedef enum smmObjGrade {
    smmObjGrade_Ap = 0,
    smmObjGrade_A0,
    smmObjGrade_Am,
    smmObjGrade_Bp,
    smmObjGrade_B0,
    smmObjGrade_Bm,
    smmObjGrade_Cp,
    smmObjGrade_C0,
    smmObjGrade_Cm
} smmObjGrade_e;


//object generation
void* smmObj_genObject(char* name, smmObjType_e objType, int type, int credit, int energy, smmObjGrade_e grade);

//member retrieving
char* smmObj_getNodeName(void* obj);
int smmObj_getNodeType(void* obj);
int smmObj_getNodeCredit(void* obj);
int smmObj_getNodeEnergy(void* obj);
smmObjGrade_e smmObj_getNodeGrade(void* obj);

//element to string
char* smmObj_getTypeName(int type);


#endif /* smm_object_h */
