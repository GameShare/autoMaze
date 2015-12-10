#include <opencv2\opencv.hpp>
#include<iostream>
#include<math.h>
#include<Windows.h>
#include"moveControl.cpp"
extern int m(int i, int j);
//#include "moveControl.cpp"
const double picktres=1;
const double reachtres=0.5;
const double rotatetres=5;
using namespace std;
using namespace cv;
//int maze[1000][1000]={};
const int dx[8]={0,-1,0,1,-1,1,-1,1};
const int dy[8]={1,0,-1,0,-1,-1,1,1};
struct Point
{
    int x,y;
    Point(int xx=0,int yy=0)
    {
        x=xx;
        y=yy;
    }
};
double abs(double x)
{
    if (x<0) return -x;
    return x;
}
struct Node
{
    int x,y,fx,fy,ava,dist;
    Node(int xx=0,int yy=0,int avain=0)
    {
        x=xx;
        y=yy;
        fx=-1;
        fy=-1;
        dist=-1;
        ava=avain;
    }
};
//Node t,s;
class Queue
{
private:
    Node *listArray;
    int front,rear,size;
public:
    Queue(int sz)
    {
        size=sz+1;
        listArray=new Node[size];
        front=1;
        rear=0;
    }
    ~Queue()
    {
        delete[] listArray;
    }
    bool enqueue(const Node &Ele)
    {
        if ((rear+2)%size==front) return false;
        rear=(rear+1)%size;
        listArray[rear]=Ele;
        return true;
    }
    bool dequeue(Node &it)
    {
        if ((rear+1)%size==front) return false;
        it=listArray[front];
        front=(front+1)%size;
        return false;
    }
    bool peek(Node &it)
    {
        if ((rear+1)%size==front) return false;
        it=listArray[front];
        return true;
    }
    bool isEmpty()
    {
        return ((rear+1)%size==front);
    }
};

bool calcMaze(int sx,int sy,int ex,int ey,int a,int b,Point2f *pList)
{
    Queue q(1000000);
    int i,j;
    Node **mazet;
    mazet=new Node*[a];
    for (i=0; i<a; i++)
        mazet[i]=new Node[b];
    for (i=0; i<a; i++)
        for (j=0; j<b; j++)
        {
            mazet[i][j]=Node(i,j,m(i,j));
        }
    Node t,s;
    mazet[sx][sy].dist=0;
    q.enqueue(mazet[sx][sy]);

    if ((m(sx,sy)==0)&&(m(ex,ey)==0))
    {
        while (!q.isEmpty())
        {
            q.dequeue(t);
            //cout<<t.x<<" "<<t.y<<endl;
            for (i=0; i<8; i++)
            {
                if ((t.x+dx[i]>=0)&&(t.x+dx[i]<a)&&(t.y+dy[i]>=0)&&(t.y+dy[i]<b)&&
                    ((m(t.x+dx[i],t.y+dy[i])==0)&&((mazet[t.x+dx[i]][t.y+dy[i]].dist==-1)||(mazet[t.x+dx[i]][t.y+dy[i]].dist>mazet[t.x][t.y].dist+1))))
                {
                    mazet[t.x+dx[i]][t.y+dy[i]].fx=t.x;
                    mazet[t.x+dx[i]][t.y+dy[i]].fy=t.y;
                    mazet[t.x+dx[i]][t.y+dy[i]].dist=mazet[t.x][t.y].dist+1;
                    q.enqueue(mazet[t.x+dx[i]][t.y+dy[i]]);
                }
            }
        }
    }
    int tx=ex,ty=ey,tempx,tempy,dir=-1,tempn=0;
    Point2f *tpl;
    tpl=new Point2f[1000000];
    while ((tx>=0)&&(ty>=0))
    {
        for (i=0; i<8; i++)
        {
            if ((tempx+dx[i]==tx)&&(tempy+dy[i]==ty))
            {
                if (dir!=i)
                {
                    dir=i;
                    //cout<<tempx<<" "<<tempy<<endl;
                    if (((tempx==ex)&&(tempy==ey))||
                    (sqrt((tempx-tpl[tempn-1].x)*(tempx-tpl[tempn-1].x)+(tempy-tpl[tempn-1].y)*(tempy-tpl[tempn-1].y))>picktres))
                    {
                        tpl[tempn].x=tempx;
                        tpl[tempn].y=tempy;
                        tempn++;
                    }
                }
                break;
            }
        }
        tempx=tx; tempy=ty;
        tx=mazet[tempx][tempy].fx;
        ty=mazet[tempx][tempy].fy;
    }
    //cout<<sx<<" "<<sy;
    tpl[tempn].x=sx;
    tpl[tempn].y=sy;
    tempn++;
    for (i=0; i<tempn; i++)
    {
        pList[i]=tpl[tempn-1-i];
    }
    for (i=0; i<tempn; i++)
    {
        cout<<pList[i].x<<" "<<pList[i].y<<endl;
    }
    delete [] tpl;
    return true;
}
bool changeDir(double headX, double headY, double tailX, double tailY,double sourX, double sourY, double targX, double targY)
{
    double midX=(headX+tailX)/2;
    double midY=(headY+tailY)/2;
    double vecCX=headX-tailX,vecCY=headY-tailY,vecRX=targX-sourX,vecRY=targY-sourY;
    double angle=asin((vecCX*vecRY-vecRX*vecCY)/(sqrt(vecCX*vecCX+vecCY*vecCY)*sqrt(vecRX*vecRX+vecRY*vecRY)));
    cout<<"the angle is: "<<angle/3.14*180<<", the command is: ";
    if (abs(angle)<rotatetres)
    {
        cout<<"Go Ahead"<<endl;
        goAhead();
    }
    else if (angle>rotatetres)
    {
        cout<<"Turn Left"<<endl;
        turnLeft();
    }
    else if (angle<-rotatetres)
    {
        cout<<"Turn Right"<<endl;
        turnRight();
    }
    //angle<0.15 Turn Right
    //abs(angle)<0.15 Go Ahead
}
