//
//  main.c
//  SMMarble
//
//  Created by Eunseo Back on 2023/12/18
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


char GradeName[9][MAX_CHARNAME] = { "A+",
                                    "A0",
                                    "A-",
                                    "B+",
                                    "B0",
                                    "B-",
                                    "C+",
                                    "C0",
                                    "C-"
                                    };
float GradeScore[9] = { 4.3,    // A+
                        4.0,    // A0
                        3.7,    // A-
                        3.3,    // B+
                        3.0,    // B0
                        2.7,    // B-
                        2.3,    // C+
                        2.0,    // C0
                        1.7     // C-
                        };
                        

#if 0
static int player_energy[MAX_PLAYER];
static int player_position[MAX_PLAYER];
static char player_name[MAX_PLAYER][MAX_CHARNAME];
#endif

//function prototypes
void printGrades(int player);   //print all the grade history of the player
void printPlayerStatus(void);   //print all player status at the beginning of each turn
char isGraduated(int player);   // check whether player's grade is bigger then GRADUATE_CREDIT or not
char isFirstLecture(int player, char* lecture_name);    // check whether took the lecture before or not
float calcAverageGrade(int player);         // calculate average grade of the player
void generatePlayers(int n, int initEnergy);    // generate a new player
int rolldie(int player);    // roll a die
void takeLecture(int player);               // Take lecture and get grade (insert a grade of the player)
void goRestaurant(int player);   // go to restaurant and charge energy
void goHome(int player);    // go home and charge energy
void goLaboratory(int player);              // go to laboratory
void goExperiment(int player);              // go Experiment
void goFoodChance(int player);   // Take food chance
void goFestival(int player);    // go Festival
void actionNode(int player);    // action code when a player stays at a node
void goForward(int player, int step);   // make player go "step" steps on the board (check if player is graduated)


#if 0

#endif


void printGrades(int player)    // print grade history of the player
{
    int i = 0;
    int list_nr = 0;
    void *lectureObj = NULL;


    // 지금까지 수강한 과목 및 점수를 보여주기.
    list_nr = smmdb_len(LISTNO_OFFSET_GRADE+player);
    
    if(list_nr <=0)
    {   
		printf("There is no lecture took...\n");
    	return;
	}


    for (i=0;i<list_nr;i++)
    {                
        lectureObj = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);
        
        printf("    --> %s took %s, credit = %i, energy = %i, grade = %s(%.1f)\n", 
                    cur_player[player].name, 
                    smmObj_getNodeName(lectureObj), 
                    smmObj_getNodeCredit(lectureObj), 
                    smmObj_getNodeEnergy(lectureObj),
                    GradeName[smmObj_getNodeGrade(lectureObj)],
                    GradeScore[smmObj_getNodeGrade(lectureObj)]);
    }   // end of for

    printf("\n");
    
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
    

    // player 중 한명이 졸업학점(GRADUATE_CREDIT) 이상의 학점을 이수했는지 확인하기
    // Y : 졸업학점 이상, N : 아직 졸업학점 이하 
    if (cur_player[player].accumCredit >= GRADUATE_CREDIT)        
        return 'Y';
    else
        return 'N';

}


char isFirstLecture(int player, char* lecture_name)     // check whether took the lecture before or not
{
    int i = 0;    
    int list_nr = 0;
    char retval = 'Y';
    void *lectureObj = NULL;


    // 이전에 수강한 과목인지 확인하기.
    list_nr = smmdb_len(LISTNO_OFFSET_GRADE+player);

    // 임시로 레코드 건수를 확인하기 위해 ....
    printf("    --> isFirstLecture ... 1, list no = %i\n", list_nr);

    for (i=0;i<list_nr;i++)
    {
        lectureObj = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);

        // 임시로 제대로 레코드를 가져왔는지 확인하기 위해 ...
        printf("    --> isFirstLecture ... 2, node %i : %s, credit = %i, energy = %i, grade = %i - %s(%.1f)\n", 
                    i,
                    smmObj_getNodeName(lectureObj), 
                    smmObj_getNodeCredit(lectureObj), 
                    smmObj_getNodeEnergy(lectureObj),
                    smmObj_getNodeGrade(lectureObj),
                    GradeName[smmObj_getNodeGrade(lectureObj)],
                    GradeScore[smmObj_getNodeGrade(lectureObj)]);

        if (strcmp(smmObj_getNodeName(lectureObj), lecture_name) == 0)
        {
            retval = 'N';
            break;
        }

    }   // end of for


    return retval;
}


float calcAverageGrade(int player)      // calculate average grade of the player
{
    int i = 0;
    int list_nr = 0;
    float cum_grade = 0.0;
    float avg_grade = 0.0;
    void *lectureObj = NULL;


    // 지금까지 수강한 과목 및 점수를 확인하고, 평균 점수 계산하기.
    list_nr = smmdb_len(LISTNO_OFFSET_GRADE+player);

    // 임시로 레코드 건수를 확인하기 위해 ....
    printf("    --> calcAverageGrade ... 1, list no = %i\n", list_nr);

    for (i=0;i<list_nr;i++)
    {
        lectureObj = smmdb_getData(LISTNO_OFFSET_GRADE+player, i);

        cum_grade += GradeScore[smmObj_getNodeGrade(lectureObj)];
        
        // 임시로 제대로 레코드를 가져왔는지 확인하기 위해 ...
        printf("    --> calcAverageGrade ... 2, node %i : %s, %i(%s), credit %i, energy %i, grade = %i - %s(%.1f)\n", 
                    i, 
                    smmObj_getNodeName(lectureObj), 
                    smmObj_getNodeType(lectureObj), 
                    smmObj_getTypeName(smmObj_getNodeType(lectureObj)),
                    smmObj_getNodeCredit(lectureObj), 
                    smmObj_getNodeEnergy(lectureObj),
                    smmObj_getNodeGrade(lectureObj),
                    GradeName[smmObj_getNodeGrade(lectureObj)],
                    GradeScore[smmObj_getNodeGrade(lectureObj)]);
    }   // end of for


    // 평균 점수 계산하기.
    if (list_nr <= 0)
        avg_grade = 0.0;
    else
        avg_grade = cum_grade / list_nr;

    return avg_grade;
}


void generatePlayers(int player_nr, int initEnergy)     // generate a new player
{
    int i = 0;

     // 입력된 player 수만큼 loop
    for (i=0;i<player_nr;i++)
    {
        //input name
        printf("Input player %i's name:", i);  
        scanf("%s", cur_player[i].name);
        fflush(stdin);
         
  
        // 초기값 설정하기
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


int rolldie(int player)     // roll a die (1 ~ 6 사이 숫자 구하기)
{
    char c;

    printf(" Press any key to roll a die (press g  to see grade): ");
    c = getchar();
    fflush(stdin);
    
    if (c == 'g' || c == 'G')
        printGrades(player);
    
    //1 ~ 6 까지 숫자가 나오도록 처리 (MAX_DIE = 6)
    return (rand()%MAX_DIE + 1);
}


void takeLecture(int player)    // Take lecture and get grade
{
    int i = 0;
    return;
}


void goRestaurant(int player)   // go to restaurant and charge energy
{
    int i = 0;
    void *boardObj;


    // 보충 에너지만큼 플레이어의 현재 에너지가 더해짐
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    cur_player[player].energy += smmObj_getNodeEnergy(boardObj);

    printf("    --> Let's eat in %s(%s) and charge %d energies (remained energy : %d)\n\n", 
                        smmObj_getNodeName(boardObj), 
                        smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                        smmObj_getNodeEnergy(boardObj), 
                        cur_player[player].energy);

    return;
}


void goHome(int player)     // go home and charge energy
{
    int i = 0;
    void *boardObj;
    

    // 지나가는 순간 지정된 보충 에너지만큼 현재 에너지에 더해짐
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    cur_player[player].energy += smmObj_getNodeEnergy(boardObj);

    printf("    --> I'm %i(%s) and charge energies %d (remained energy : %d)\n\n", 
                    cur_player[player].position,
                    smmObj_getNodeName(boardObj),
                    smmObj_getNodeEnergy(boardObj),
                    cur_player[player].energy);

    return;
}


void goLaboratory(int player)   // go to laboratory
{
    int i = 0;
    int die_result = 0;
    void *boardObj;


    // 실험중 상태면 주사위를 굴려서 사전에 지정된 실험 성공 기준값 이상이 나오면 실험이 종료되고, 
    // 그렇지 않으면 이동하지 못하고 실험중 상태로 머무름. 
    // (단순히 실험실 노드에 머무른다고 실험중 상태가 되지 않으며, 실험 노드에 머물러야 실험중 상태가 됨)
    // 실험 시도마다 소요 에너지만큼 소요하며, 이로 인해 에너지가 음수가 될 수 있음
    
    if (cur_player[player].flag_experiment != 'Y')
    {
        // 실험중 상태가 아니므로, skip 
        printf("   -> I'm not in experimenting status ... skip this !!\n\n");        
        return;
    }
    

    // 실험중 상태이므로, 주사위를 다시 굴려 지정된 실험 성공 기준값 이상이 나오는지 확인.
    printf("    --> I'm still experimenting at laboratory ... \n");

    die_result = rolldie(player);
    printf("    --> %s got %d\n", cur_player[player].name, die_result);

    if (die_result >= cur_player[player].experiment_target)
    {
        printf("    --> Experiment result : %d, success!!  %s can exit this lab!\n",
                    die_result, cur_player[player].name);
    
        cur_player[player].flag_experiment = 'N';       // 실험중 아님으로 변경
        cur_player[player].experiment_target = 0;       // 실험 성공 기준값 초기화
    }
    else
    {
        printf("    --> Experiment result : %d, fail T_T. %s needs more experiment......\n",
                    die_result, cur_player[player].name);
    }


    // 실험 시도마다 소요 에너지만큼 소요
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    cur_player[player].energy -= smmObj_getNodeEnergy(boardObj);
    printf("    --> %s consumes %d energy ...\n\n", 
                smmObj_getNodeName(boardObj),
                smmObj_getNodeEnergy(boardObj));

    return;
}

void goExperiment(int player)   // go Experiment
{
    int i = 0;
    int position = 0;
    int experiment_target = 0;
    char flag_found = 'N';
    void *boardObj;


    // 실험중 상태로 전환되면서 실험실로 이동 
    // (주사위 눈 범위에서 실험 성공 기준값을 랜덤으로 지정)
    
    // 실험실 Node 찾기
    position = 0;
    flag_found = 'N';
    for (i=0;i<board_nr;i++)   // board 갯수만큼
    {
        boardObj = smmdb_getData(LISTNO_NODE, i);

        if (smmObj_getNodeType(boardObj) == laboratory)     // laboratory (2)
        {   
            // 실험실 노드에 도착헸으면, 루핑 종료 
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
    
    
    // 지정된 위치(실험실)로 이동된 위치 기록
    cur_player[player].position = position;

    // 목표 주사위 숫자를 랜덤하게 지정 (1 ~ 6)  -> 4 로 지정
    // experiment_target = rand()%MAX_DIE + 1;
    experiment_target = 4;
    
    cur_player[player].flag_experiment = 'Y';                   // 실험중 상태로 변경
    cur_player[player].experiment_target = experiment_target;   // 실험 성공 기준값 설정


    printf("    --> Experiment time! Let's see if you can satisfy professor (threshold: 4)\n");
    printf("    --> jump to %d(%s), experiment Y/N = %c (target no = %d)\n\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj),
                    cur_player[player].flag_experiment,
                    cur_player[player].experiment_target);


    // go to laboratory
    goLaboratory(player);

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


    // 음식카드를 한장 랜덤으로 고르고 명시된 보충 에너지를 현재 에너지에 더함
    // food cards 갯수(전역변수 = food_nr) 까지 숫자가 나오도록 처리
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


    // 축제카드를 한장 랜덤으로 골라서 명시된 미션을 수행
    // festival cards 갯수(전역변수 = festival_nr) 까지 숫자가 나오도록 처리
    festival_card = rand()%festival_nr;  

    festivalObj = smmdb_getData(LISTNO_FESTIVAL_CARD, festival_card);



    printf("    --> Mission = %s !!\n", smmObj_getNodeName(festivalObj));
    printf("        (press any key when mission is ended)\n\n");

    fflush(stdin);
    c = getchar();
    
    return;
}


void actionNode(int player)     // action code when a player stays at a node
{
    int i = 0;
    int type = 0;       // 임시로 사용
    int credit = 0;
    int energy = 0;
    char node_name[MAX_CHARNAME];
    char type_name[MAX_CHARNAME];
    void *boardObj;
    void *gradeObj;



    if (cur_player[player].flag_completion == 'Y')
    {
        // 게임 종료 조건을 만족시킨 player 는 아래 action 수행하지 않음
        return;
    }


    // 현재 위치한 node type 에 따른, Action 수행하기
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf("    --> ActionNode : %s, type = %d(%s), credit = %i, energy = %i\n", 
                    smmObj_getNodeName(boardObj),
					smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj),
                    smmObj_getNodeEnergy(boardObj));  


    type = experiment;

    // switch(type)
    switch(smmObj_getNodeType(boardObj))    
    {
    	case lecture:   // type = 0
    	// 현재 에너지가 소요에너지 이상 있고 이전에 듣지 않은 강의이면 수강 가능하며, 
    	// 수강 혹은 드랍을 선택할 수 있음. 
    	// (수강하면 성적이 A+, A0, A-, B+, B0, B-, C+, C0, C- 중 하나가 랜덤으로 나옴)
    		takeLecture(player);
    		break;
    
        case restaurant:        // type = 1
            // 보충 에너지만큼 플레이어의 현재 에너지가 더해짐
            goRestaurant(player);
            break;
        
		case laboratory:        // type = 2
    	// 실험중 상태면 주사위를 굴려서 사전에 지정된 실험 성공 기준값 이상이 나오면 실험이 종료되고, 
    	// 그렇지 않으면 이동하지 못하고 실험중 상태로 머무름. 
    	// (단순히 실험실 노드에 머무른다고 실험중 상태가 되지 않으며, 실험 노드에 머물러야 실험중 상태가 됨)
    	// 실험 시도마다 소요 에너지만큼 소요하며, 이로 인해 에너지가 음수가 될 수 있음
    	goLaboratory(player);
    	break;
	   
        case home:              // type = 3
            // 지나가는 순간 지정된 보충 에너지만큼 현재 에너지에 더해짐
            goHome(player);
            break;

        case experiment:       // type = 4
            // 실험중 상태로 전환되면서 실험실로 이동 
            // (주사위 눈 범위에서 실험 성공 기준값을 랜덤으로 지정)
            goExperiment(player);
            break;

        case foodChance:        // type = 5
            // 음식카드를 한장 랜덤으로 고르고 명시된 보충 에너지를 현재 에너지에 더함
            goFoodChance(player);
            break;

        case festival:          // type = 6
            // 축제카드를 한장 랜덤으로 골라서 명시된 미션을 수행
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
    void *boardObj;


    if (step == 0)
    {
        // 위치 이동 없음
        return;
    }


    // 현재 node 위치(index)에서, 주어진 step 만큼 이동하기
    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);
    printf("    --> current position = %d(%s), type = %d(%s)\n", 
                cur_player[player].position, 
                smmObj_getNodeName(boardObj), 
                smmObj_getNodeType(boardObj),
                smmObj_getTypeName(smmObj_getNodeType(boardObj)));


    // step 만큼 한칸씩 이동 ...
    for (i=1;i<=step;i++)
    {    
        cur_player[player].position += 1;

        if (cur_player[player].position >= board_nr)     // board 갯수보다 
        {
            // 현 위치가 마지막 board 위치(board 갯수) >= 이면, 처음 위치로 이동 
            cur_player[player].position = 0;
        }


        // 신규 board 위치에 대한 정보 확인하기
        boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);
        
        printf("    --> jump to %i : %s, type = %i(%s), credit = %i, energy = %i\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj), 
                    smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj), 
                    smmObj_getNodeEnergy(boardObj));


        if (smmObj_getNodeType(boardObj) == home)   // type = 3 (우리집)
        {
            if (isGraduated(player) == 'Y')
            {
                // 졸업 학점을 채우고, 집으로 이동하면 게임이 즉시 종료됨
                // 게임 종료 대상임을 기록함 (completion_flag)
                cur_player[player].flag_completion = 'Y';
                printf("    --> I'm home and get a graduate credit ...\n");
                break;      // <-- 다음 노드로 추가 이동하지 않도록 처리.
            }

            if (i != step)      // 마지막 이동 위치가 아닌 경우
            {
                // 에너지 보충하기
                goHome(player);
            }
        }      

    }   // end of for


    boardObj = smmdb_getData(LISTNO_NODE, cur_player[player].position);

    printf("    --> new posistion = %d(%s), type = %d(%s), credit = %d, energy = %d\n\n", 
                    cur_player[player].position, 
                    smmObj_getNodeName(boardObj), 
                    smmObj_getNodeType(boardObj), 
                    smmObj_getTypeName(smmObj_getNodeType(boardObj)),
                    smmObj_getNodeCredit(boardObj), 
                    smmObj_getNodeEnergy(boardObj));

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
    
    system("chcp 65001");       //gcc 한글 깨짐 방지를 위해, 코드 추가

   
    //1. import parameters ---------------------------------------------------------------------------------
    //1-1. boardConfig (marbleBoardConfig.txt 파일 읽기)
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
        
        if (type == home)   // type = 3 (우리집) 
           initEnergy = energy;

        board_nr++;
    }   // end of while
    fclose(fp);
    printf("Total number of board nodes : %i\n", board_nr);
    
    //  위에서 읽은 board config 정보 확인하기
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

    //  위에서 읽은 food config 정보 확인하기
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

    //  위에서 읽은 festival config 정보 확인하기
    for (i=0;i<festival_nr;i++)
    {
        festivalObj = smmdb_getData(LISTNO_FESTIVAL_CARD, i);
        
        printf("node %i : %s\n", 
                    i, 
                    smmObj_getNodeName(festivalObj));
    }   // end of for
    // 여기까지 board, food, festival config 파일 읽고, database에 저장하기
    
    

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

        // atoi 함수 : 문자열이 숫자이면 숫자로 변환해 주고, 숫자가 아니면 0 값을 반환
        player_nr = atoi(temp);

    }
    while (player_nr < 1 || player_nr >  MAX_PLAYER);   // 1 ~ 10 사이의 숫자를 입력할 때까지, 반복
    
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
        
        if (cur_player[turn].flag_experiment == 'Y')    // 실험중 상태
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
        // 게임 종료 조건을 만족시킨 player 가 있으면, 바로 게임 종료하기.
        // player 중 한명이 GRADUATE_CREDIT 이상의 학점(30점)을 이수하고, 
        // 집으로 이동하면 게임 즉시 종료
        if (cur_player[turn].flag_completion == 'Y')
        {
            printf("    --> %s is winner of this game !!\n\n", cur_player[turn].name);

            // 게임 기간 수강한 과목 및 점수를 프린트하기.
            printGrades(turn);
            break;  // while loop 종료
        }


        turn = (turn + 1)%player_nr;

        if (turn == 0)
        {
            max_try_cnt++;
            printf("Try Count = [%d]\n", max_try_cnt);
        }

    }   // end of while
        
    printf("\n\n");

    free(cur_player);
    system("PAUSE");
    return 0;
}
