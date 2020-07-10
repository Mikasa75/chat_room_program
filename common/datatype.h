#ifndef _DATATYPE_H

#define _DATATYPE_H



#define MAX 300 //�û����Ķ���֮һ

#define NTHREAD 3



struct Score {

    int red;

    int blue;

};

struct Bpoint {

    double x;

    double y;

};



struct Speed {

    double x;

    double y;

};



struct Aspeed {

    double x;

    double y;

};



struct BallStatus {

    struct Speed v;

    struct Aspeed a;

    int who;//which����

    char name[20];

};



struct Point {

    int x;

    int y;

};

struct User {

    int team; // 0 RED  1 BLUE

    int fd; //����Ҷ�Ӧ������

    char name[20];

    int online;// 1 ���� 0 ������

    int flag; //δ��Ӧ����

    struct Point loc;

};





//��¼��ص�

struct  LogRequest {

    char name[20];

    int team;

    char msg[512];

};



struct LogResponse {

    int type; // 0 OK 1 NO

    char msg[512];

};

//��Ϸ�ڼ佻��

#define MAX_MSG 4096

//�ճ�����Ϣ��������������Ϣ��ͳһΪ512����



#define ACTION_KICK 0x01

#define ACTION_CARRY 0x02

#define ACTION_STOP 0x04



struct Ctl {

    int action;

    int dirx;

    int diry;

    int strength; //��������

};



#define FT_HEART 0x01 //����

#define FT_WALL 0x02 //����

#define FT_MSG 0x04 //����

#define FT_ACK 0x08 //ack

#define FT_CTL 0x10 //������Ϣ

#define FT_GAME 0x20 //��Ϸ��������

#define FT_SCORE 0x40 //�ȷֱ仯

#define FT_GAMEOVER 0x80 //gameover

#define FT_FIN 0x100 //�볡



struct FootBallMsg {

    int type;  // type & FT_HEART

    int size;

    int team;

    char name[20];

    char msg[MAX_MSG];

    struct Ctl ctl;

};

#define CHAT_FIN 0x01
#define CHAT_HERT 0x02
#define CHAT_ACK 0x04
#define CHAT_WALL 0x08
#define CHAT_MSG 0x10
#define CHAT_FUNC 0x20
#define CHAT_SYS 0x40
struct ChatMsg {
    int type;
    char msg[1024];
    char name[20];
};



struct Map {

    int width;

    int height;

    struct Point start;

    int gate_width;

    int gate_height;

};

#endif
