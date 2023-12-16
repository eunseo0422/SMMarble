//
//  main.c
//  SMMarble
//
//  Created by Eunseo Back on 2023/11/05.
//

#include <time.h>
#include <string.h>
#include "smm_object.h"
#include "smm_database.h"
#include "smm_common.h"

#define BOARDFILEPATH "marbleBoardConfig.txt"
#define FOODFILEPATH "marbleFoodConfig.txt"
#define FESTFILEPATH "marbleFestivalConfig.txt"



//board configuration parameters
static int board_nr;
static int food_nr;
static int festival_nr;

static int player_nr;


typedef struct player {
        char name[MAX_CHARNAME];
        int position;
        int energy;
        int accumCredit;
        char flag_graduate;
        char flag_experiment;
        int experiment_target;
        char flag_completion;
        
} player_t;

static player_t *cur_player;


typedef struct TakeLecture {
    char node_name[MAX_CHARNAME];
    char grade_name[MAX_CHARNAME];
    float grade_score;
    int credit;
    int energy;
} TakeLecture_t;



#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
void printGrades(int player);   //print all the grade history of the player
void printPlayerStatus(void);   //print all player status at the beginning of each turn
char isGraduated(int player);   // check whether player's grade is bigger then GRADUATE_CREDIT or not
void generatePlayers(int n, int initEnergy);    // generate a new player
int rolldie(int player);    // roll a die
void goRestaurant(int player, char *node_name, int energy);   // go to restaurant and charge energy
void goHome(int player, int energy);    // go home and charge energy
void goFoodChance(int player);   // Take food chance
void goFestival(int player);    // go Festival
void actionNode(int player);    // action code when a player stays at a node
void goForward(int player, int step);   // make player go "step" steps on the board (check if player is graduated)

#if 0
int isGraduated(void); //check if any player is graduated
float calcAverageGrade(int player); //calculate average grade of the player
smmGrade_e takeLecture(int player, char *lectureName, int credit); //take the lecture (insert a grade of the player)
void* findGrade(int player, char *lectureName); //find the grade from the player's grade history

#endif


void printGrades(int player)    // print grade history of the player
{
    int i = 0;
    int list_nr = 0;
    int lecture_nr = 0;
    float grade_cum = 0.0;
    float grade_avg = 0.0;
    TakeLecture_t* obj_record = NULL;


    // ���ݱ��� ������ ���� �� ������ �����ֱ�.
    list_nr = smmdb_len(player);
    printf("printGrades - mile stone 1 ... list count = [%d]\n", list_nr);

    for (i=0;i<list_nr;i++)
    {                
        obj_record = smmdb_getData(player, i);
        
        lecture_nr++;
        grade_cum  += obj_record->grade_score;

        printf("    --> %s took %s with grade [%s]-[%.6f], credit=[%d], energy=[%d] \n", 
                cur_player[player].name,
                obj_record->node_name, 
                obj_record->grade_name,
                obj_record->grade_score, 
                obj_record->credit, 
                obj_record->energy);    

    }   // end of for

    return;
}

void printPlayerStatus(void)    // print all player status at the beginning of each turn
{
    int i;
    void *boardObj;
    
    
    printf("\n\n");
    printf("=================================== PLAYER STATUS =============================================\n");

    for (i=0;i<player_nr;i++)    
    {
        boardObj = smmdb_getData(LISTNO_NODE, cur_player[i].position);

        printf("%i - %s : credit = %i, energy = %i, position = %i(%s:%s), graduate = %c, experiment = %c(%i)\n", 
                    i,
                    cur_player[i].name,
                    cur_player[i].accumCredit,
                    cur_player[i].energy,
                    cur_player[i].position,
                    smmObj_getNodeName(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    cur_player[i].flag_graduate,
                    cur_player[i].flag_experiment,
                    cur_player[i].experiment_target);
    }   // end of for

    printf("===============================================================================================\n");
    printf("\n\n");

    return;
}


char isGraduated(int player)    // check whether player's grade is bigger then GRADUATE_CREDIT or not
{
    int i = 0;
    

    // player �� �Ѹ��� ��������(GRADUATE_CREDIT) �̻��� ������ �̼��ߴ��� Ȯ���ϱ�
    // Y : �������� �̻�, N : ���� �������� ���� 
    if (cur_player[player].accumCredit >= GRADUATE_CREDIT)        
        return 'Y';
    else
        return 'N';

}


void generatePlayers(int player_nr, int initEnergy)     // generate a new player
{
    int i = 0;

     // �Էµ� player ����ŭ loop
    for (i=0;i<player_nr;i++)
    {
        //input name
        printf("Input player %i's name:", i);  
        scanf("%s", cur_player[i].name);
        fflush(stdin);
         
  
        // �ʱⰪ �����ϱ�
        cur_player[i].position = 0;
        cur_player[i].energy = initEnergy;
        cur_player[i].accumCredit = 0;
        cur_player[i].flag_graduate = 'N';
        cur_player[i].flag_experiment = 'N';
        cur_player[i].experiment_target = 0;
        cur_player[i].flag_completion = 'N';

    }   // end of for

    return;
}


int rolldie(int player)     // roll a die (1 ~ 6 ���� ���� ���ϱ�)
{
    char c;

    printf(" Press any key to roll a die (press g  to see grade): ");
    c = getchar();
    fflush(stdin);
    
    if (c == 'g' || c == 'G')
        printGrades(player);
    
    //1 ~ 6 ���� ���ڰ� �������� ó�� (MAX_DIE = 6)
    return (rand()%MAX_DIE + 1);
}


void goRestaurant(int player, char *node_name, int energy)   // go to restaurant and charge energy
{
    int i = 0;


    // ���� ��������ŭ �÷��̾��� ���� �������� ������
    cur_player[player].energy += energy;

    printf("    --> Let's eat in %s and charge %d energies (remained energy : %d)\n\n", 
                        node_name, energy, cur_player[player].energy);

    return;
}


void goHome(int player, int energy)     // go home and charge energy
{
    int i = 0;
    void *boardObj;
    

    // �������� ���� ������ ���� ��������ŭ ���� �������� ������
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    cur_player[player].energy += energy;

    printf("    --> I'm %i(%s) and charge energies %d (remained energy : %d)\n", 
                    cur_player[player].position,
                    smmObj_getNodeName(boardObj),
                    energy, 
                    cur_player[player].energy);
    printf("\n");

    return;
}


void goExperiment(int player)   // go Experiment
{
    int i = 0;
    int position = 0;
    int experiment_target = 0;
    char flag_found = 'N';
    void *boardObj;


    // ������ ���·� ��ȯ�Ǹ鼭 ����Ƿ� �̵� 
    // (�ֻ��� �� �������� ���� ���� ���ذ��� �������� ����)
    
    // ����� Node ã��
    position = 0;
    flag_found = 'N';
    for (i=0;i<board_nr;i++)   // board ������ŭ
    {
        boardObj = smmdb_getData(LISTNO_NODE, i);

        if (smmObj_getNodeType(boardObj) == laboratory)     // laboratory (2)
        {   
            // ����� ��忡 �����g����, ���� ���� 
            flag_found = 'Y';
            position = i;
            break;
        }
    }   // end of for


    if (flag_found == 'N')
    {
        printf("    --> Fail to laboratory node ... !!! \n\n");
        return;
    }
    
    
    // ������ ��ġ(�����)�� �̵��� ��ġ ���
    cur_player[player].position = position;

    // ��ǥ �ֻ��� ���ڸ� �����ϰ� ���� (1 ~ 6)
    experiment_target = rand()%MAX_DIE + 1;
    
    cur_player[player].flag_experiment = 'Y';                   // ������ ���·� ����
    cur_player[player].experiment_target = experiment_target;   // ���� ���� ���ذ� ����


    printf("    --> Experiment time! Let's see if you can satisfy professor (threshold: 4)\n");
    printf("    --> jump to %d(%s), experiment Y/N = %c (target no = %d)\n\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj),
                    cur_player[player].flag_experiment,
                    cur_player[player].experiment_target);


    // go to laboratory
    // goLaboratory(player, nth, energy);

    return;
}


void goFoodChance(int player)   // Take food chance
{
    int i = 0;
    int food_card = 0;   
    char c;
    void *foodObj;
    
    
    printf("    --> %s has a chance to take food! press any key to pick a food card: ",
                    cur_player[player].name);
    c = getchar();
    fflush(stdin);


    // ����ī�带 ���� �������� ���� ��õ� ���� �������� ���� �������� ����
    // food cards ����(�������� = food_nr) ���� ���ڰ� �������� ó��
    food_card = rand()%food_nr;

    foodObj = smmdb_getData(LISTNO_FOOD_CARD, food_card);

    cur_player[player].energy += smmObj_getNodeEnergy(foodObj);
           
    printf("    -> %s takes food chance = %s, energy = %d, remained energy = %d\n\n", 
                    cur_player[player].name, 
                    smmObj_getNodeName(foodObj), 
                    smmObj_getNodeEnergy(foodObj),
                    cur_player[player].energy);

    return;
}



void goFestival(int player)     // go Festival
{
    int i = 0;
    char c;
    int festival_card = 0;
    void *festivalObj; 


    printf("    --> %s participates to Snow Festival! press any key to pick a festival card: ",
                    cur_player[player].name); 
    fflush(stdin);
    c = getchar();


    // ����ī�带 ���� �������� ��� ��õ� �̼��� ����
    // festival cards ����(�������� = festival_nr) ���� ���ڰ� �������� ó��
    festival_card = rand()%festival_nr;  

    festivalObj = smmdb_getData(LISTNO_FESTIVAL_CARD, festival_card);



    printf("    --> Mission = %s !!\n", smmObj_getNodeName(festivalObj));
    printf("        (press any key when mission is ended)\n\n");

    fflush(stdin);
    c = getchar();
    
    printf("\n");

    return;
}


void actionNode(int player)     // action code when a player stays at a node
{
    int i = 0;
    int type = 0;       // �ӽ÷� ���
    int credit = 0;
    int energy = 0;
    char node_name[MAX_CHARNAME];
    char type_name[MAX_CHARNAME];
    void *boardObj;
    void *gradeObj;



    if (cur_player[player].flag_completion == 'Y')
    {
        // ���� ���� ������ ������Ų player �� �Ʒ� action �������� ����
        return;
    }


    // ���� ��ġ�� node type �� ����, Action �����ϱ�
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf("    --> ActionNode : %s, type = %d(%s), credit = %i, energy = %i\n", 
                    smmObj_getNodeName(boardObj),
					smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj),
                    smmObj_getNodeEnergy(boardObj));  


    type = festival;

    // switch(type)
    switch(smmObj_getNodeType(boardObj))    
    {
        case restaurant:        // type = 1
            // ���� ��������ŭ �÷��̾��� ���� �������� ������
            // goRestaurant(player, smmObj_getTypeName(smmObj_getNodeType(boardObj)), smmObj_getNodeEnergy(boardObj));
            goRestaurant(player, smmObj_getNodeName(boardObj), smmObj_getNodeEnergy(boardObj));
            break;
            
        case home:              // type = 3
            // �������� ���� ������ ���� ��������ŭ ���� �������� ������
            goHome(player, smmObj_getNodeEnergy(boardObj));
            break;

        case experiment:       // type = 4
            // ������ ���·� ��ȯ�Ǹ鼭 ����Ƿ� �̵� 
            // (�ֻ��� �� �������� ���� ���� ���ذ��� �������� ����)
            goExperiment(player);
            break;

        case foodChance:        // type = 5
            // ����ī�带 ���� �������� ���� ��õ� ���� �������� ���� �������� ����
            goFoodChance(player);
            break;

        case festival:          // type = 6
            // ����ī�带 ���� �������� ��� ��õ� �̼��� ����
            goFestival(player);
            break;

        default:
            break;
    }   // end of switch

    return;
}


void goForward(int player, int step)    // make player go "step" steps on the board (check if player is graduated)
{
    int i = 0;
    int type = 0;
    char type_name[MAX_CHARNAME];
    void *boardObj;


    if (step == 0)
    {
        // ��ġ �̵� ����
        return;
    }


    // ���� node ��ġ(index)����, �־��� step ��ŭ �̵��ϱ�
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    printf("    --> current position = %d(%s), type = %d(%s)\n", 
                cur_player[player].position, 
                smmObj_getNodeName(boardObj), 
                smmObj_getNodeType(boardObj),
                smmObj_getTypeName(smmObj_getNodeType(boardObj)));


    // step ��ŭ ��ĭ�� �̵� ...
    for (i=1;i<=step;i++)
    {    
        cur_player[player].position += 1;

        if (cur_player[player].position >= board_nr)     // board �������� 
        {
            // �� ��ġ�� ������ board ��ġ(board ����) >= �̸�, ó�� ��ġ�� �̵� 
            cur_player[player].position = 0;
        }


        // �ű� board ��ġ�� ���� ���� Ȯ���ϱ�
        boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);
        
        printf("    --> jump to %i : %s, type = %i(%s), credit = %i, energy = %i\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj), 
                    smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj), 
                    smmObj_getNodeEnergy(boardObj));



        type = smmObj_getNodeType(boardObj);
        strcpy(type_name, smmObj_getTypeName(smmObj_getNodeType(boardObj)));

        if (type == home)   // type = 3 (�츮��)
        {
            if (isGraduated(player) == 'Y')
            {
                // ���� ������ ä���, ������ �̵��ϸ� ������ ��� �����
                // ���� ���� ������� ����� (completion_flag)
                cur_player[player].flag_completion = 'Y';
                printf("    --> I'm home and get a graduate credit ...\n");
                break;      // <-- ���� ���� �߰� �̵����� �ʵ��� ó��.
            }

            if (i != step)      // ������ �̵� ��ġ�� �ƴ� ���
            {
                // ������ �����ϱ�
                goHome(player, smmObj_getNodeEnergy(boardObj));
            }
        }      

    }   // end of for


    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf("    --> new posistion = %d(%s), type = %d(%s), credit = %d, energy = %d\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj), 
                    smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj), 
                    smmObj_getNodeEnergy(boardObj));
    printf("\n\n");

    return;
}


int main(int argc, const char * argv[]) {
    
    FILE* fp;
    char name[MAX_CHARNAME];
    char temp[MAX_CHARNAME];
    int type;
    int credit;
    int energy;
    int i;
    int initEnergy;
    int turn = 0;
    int max_try_cnt = 0;
    char flag_anybody_graduated = 'N';
    void *boardObj;
    void *foodObj;
    void *festivalObj;
    

    board_nr = 0;
    food_nr = 0;
    festival_nr = 0;
    
    srand(time(NULL));
    
    system("chcp 65001");       //gcc �ѱ� ���� ������ ����, �ڵ� �߰�

   
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig (marbleBoardConfig.txt ���� �б�)
    if ((fp = fopen(BOARDFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", BOARDFILEPATH);
        getchar();
        return -1;
    }
    
    printf("Reading board component......\n");
    while ( fscanf(fp, "%s %i %i %i", name, &type, &credit, &energy) != EOF ) //read a node parameter set
    {
        //store the parameter set
        boardObj = smmObj_genObject(name, smmObjType_board, type, credit, energy, 0);
        smmdb_addTail(LISTNO_NODE, boardObj);
        
        if (type == home)   // type = 3 (�츮��) 
           initEnergy = energy;

        board_nr++;
    }   // end of while
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    //  ������ ���� board config ���� Ȯ���ϱ�
    for (i=0;i<board_nr;i++)
    {
        boardObj = smmdb_getData(LISTNO_NODE, i);
        
        printf("node %i : %s, %i(%s), credit %i, energy %i\n", 
                    i, 
                    smmObj_getNodeName(boardObj), 
                    smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj), 
                    smmObj_getNodeEnergy(boardObj));
    }   // end of for
    printf("    --> check the value of SMMNODE_TYPE_LECTURE = (%s)\n\n", smmObj_getTypeName(lecture));
    


    //2. food card config 
    if ((fp = fopen(FOODFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FOODFILEPATH);
        return -1;
    }
    
    printf("\n\nReading food card component......\n");
    while ( fscanf(fp, "%s %i", name, &energy) != EOF ) //read a food parameter set
    {
        //store the parameter set
        foodObj = smmObj_genObject(name, smmObjType_food, 0, 0, energy, 0);
        smmdb_addTail(LISTNO_FOOD_CARD, foodObj);

        food_nr++;
    }   // end of while
    fclose(fp);
    printf("Total number of food cards : %i\n", food_nr);

    //  ������ ���� food config ���� Ȯ���ϱ�
    for (i=0;i<food_nr;i++)
    {
        foodObj = smmdb_getData(LISTNO_FOOD_CARD, i);
        
        printf("node %i : %s, energy %i\n", 
                    i, 
                    smmObj_getNodeName(foodObj), 
                    smmObj_getNodeEnergy(foodObj));
    }   // end of for
    
    
    
    //3. festival card config 
    if ((fp = fopen(FESTFILEPATH,"r")) == NULL)
    {
        printf("[ERROR] failed to open %s. This file should be in the same directory of SMMarble.exe.\n", FESTFILEPATH);
        return -1;
    }
    
    printf("\n\nReading festival card component......\n");
    while ( fscanf(fp, "%s", name) != EOF ) //read a festival card string
    {
        //store the parameter set
        festivalObj = smmObj_genObject(name, smmObjType_festival, 0, 0, 0, 0);
        smmdb_addTail(LISTNO_FESTIVAL_CARD, festivalObj);

        festival_nr++;
    }   // end of while
    fclose(fp);
    printf("Total number of festival cards : %i\n", festival_nr);

    //  ������ ���� festival config ���� Ȯ���ϱ�
    for (i=0;i<festival_nr;i++)
    {
        festivalObj = smmdb_getData(LISTNO_FESTIVAL_CARD, i);
        
        printf("node %i : %s\n", 
                    i, 
                    smmObj_getNodeName(festivalObj));
    }   // end of for
    // ������� board, food, festival config ���� �а�, database�� �����ϱ�
    
    

    printf("\n\n\n\n");
    printf("=======================================================================\n");
    printf("-----------------------------------------------------------------------\n");
    printf("      Sookmyung Marble !! Let's Graduate (total credit : 30)!!         \n");
    printf("-----------------------------------------------------------------------\n");
    printf("=======================================================================\n");
    printf("\n\n\n\n");

    // 2. Player configuration --------------------------------------------------------------------------------    
    do
    {
        //input player number to player_nr
        printf("input player no. (1 ~ 10):");
        scanf("%s", temp);
        fflush(stdin);

        // atoi �Լ� : ���ڿ��� �����̸� ���ڷ� ��ȯ�� �ְ�, ���ڰ� �ƴϸ� 0 ���� ��ȯ
        player_nr = atoi(temp);

    }
    while (player_nr < 1 || player_nr >  MAX_PLAYER);   // 1 ~ 10 ������ ���ڸ� �Է��� ������, �ݺ�
    
    cur_player = (player_t*)malloc(player_nr*sizeof(player_t));

    generatePlayers(player_nr, initEnergy);
    
    
    
    //3. SM Marble game starts ---------------------------------------------------------------------------------
    max_try_cnt = 0;
    flag_anybody_graduated = 'N';

    while (flag_anybody_graduated == 'N')   // is anybody graduated?
    {
        int die_result;
        
        
        // 4-1. initial printing
        printPlayerStatus();
        

        // 4-2. die rolling (if not in experiment)    
        printf("This is %s's turn :::: ", cur_player[turn].name);
        
        if (cur_player[turn].flag_experiment == 'Y')    // ������ ����
        {
            die_result = 0;
        }
        else
        {
            die_result = rolldie(turn);
            printf("    --> %s got %d\n", cur_player[turn].name, die_result);
        }    
        

        // 4-3. go forward
        goForward(turn, die_result);


		// 4-4. take action at the destination node of the board
        actionNode(turn);
        

        // 4-5. next turn    
        // ���� ���� ������ ������Ų player �� ������, �ٷ� ���� �����ϱ�.
        // player �� �Ѹ��� GRADUATE_CREDIT �̻��� ����(30��)�� �̼��ϰ�, 
        // ������ �̵��ϸ� ���� ��� ����
        if (cur_player[turn].flag_completion == 'Y')
        {
            printf("    --> %s is winner of this game !!\n\n", cur_player[turn].name);

            // ���� �Ⱓ ������ ���� �� ������ ����Ʈ�ϱ�.
            printGrades(turn);
            break;  // while loop ����
        }


        turn = (turn + 1)%player_nr;

        if (turn == 0)
        {
            max_try_cnt++;
            printf("Try Count = [%d]\n", max_try_cnt);
        }
        

        if (max_try_cnt >= 5)   // �ӽ÷� player �� 5ȸ�� ����
            break;    // while loop ����

    }   // end of while
        
    printf("\n\n");

    free(cur_player);
    system("PAUSE");
    return 0;
}
