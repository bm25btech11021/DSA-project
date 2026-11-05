
#include<bits/stdc++.h>
using namespace std;
string toLower(string s){
    for(int i=0;i<(int)s.size();i++)
        s[i]=tolower((unsigned char)s[i]);
    return s;
}
bool isBlank(string s){
    for(int i=0;i<(int)s.size();i++)
        if(!isspace((unsigned char)s[i]))
            return false;
    return true;
}
struct Team{
    int id=0;
    string name,captain;
    bool hadBye=false;
    int bracketNode=-1;
    string exitNote;
    Team* next=nullptr;
};
class TeamList{
public:
    Team* head=nullptr;
    Team* tail=nullptr;
    int count=0;
    TeamList(const TeamList&)=delete;
    TeamList& operator=(const TeamList&)=delete;
    ~TeamList(){
        Team* current=head;
        while(current!=nullptr){
            Team* temp=current;
            current=current->next;
            delete temp;
        }
    }
    bool exists(string name){
        Team* current=head;
        while(current!=nullptr){
            if(toLower(current->name)==toLower(name))
                return true;
            current=current->next;
        }
        return false;
    }
    Team* add(int id,string name,string captain){
        Team* t=new Team;
        t->id=id;
        t->name=name;
        t->captain=captain;
        if(head==nullptr)
            head=tail=t;
        else{
            tail->next=t;
            tail=t;
        }
        count++;
        return t;
    }
    vector<Team*> toVector(){
        vector<Team*> v;
        Team* current=head;
        while(current!=nullptr){
            v.push_back(current);
            current=current->next;
        }
        return v;
    }
    void print(){
        if(head==nullptr){
            cout<<"No teams registered.\n";
            return;
        }
        Team* current=head;
        while(current!=nullptr){
            cout<<"ID: "<<current->id<<" | Team: "<<current->name<<" | Captain: "<<current->captain<<"\n";
            current=current->next;
        }
    }
};
enum MatchType{NORMAL,BRONZE,FINAL_MATCH,PLACEMENT};
struct Match{
    int id=0;
    Team* a=nullptr;
    Team* b=nullptr;
    MatchType type=NORMAL;
    string stage;
    int pairIdx=-1,nodeIdx=-1,replays=0;
};
struct BracketNode{
    string text;
    int left=-1,right=-1;
};
template<class T>
class MyQueue{
private:
    struct Node{
        T data;
        Node* next;
        Node(const T& value){
            data=value;
            next=nullptr;
        }
    };
    Node* frontNode=nullptr;
    Node* rearNode=nullptr;
    int count=0;
    void clear(){
        while(frontNode!=nullptr){
            Node* temp=frontNode;
            frontNode=frontNode->next;
            delete temp;
        }
        rearNode=nullptr;
        count=0;
    }
public:
    MyQueue(){}
    MyQueue(const MyQueue& other){
        Node* current=other.frontNode;
        while(current!=nullptr){
            push(current->data);
            current=current->next;
        }
    }
    MyQueue& operator=(const MyQueue& other){
        if(this!=&other){
            clear();
            Node* current=other.frontNode;
            while(current!=nullptr){
                push(current->data);
                current=current->next;
            }
        }
        return *this;
    }
    ~MyQueue(){
        clear();
    }
    bool empty() const{
        return count==0;
    }
    int size() const{
        return count;
    }
    void push(const T& value){
        Node* temp=new Node(value);
        if(rearNode==nullptr)
            frontNode=rearNode=temp;
        else{
            rearNode->next=temp;
            rearNode=temp;
        }
        count++;
    }
    void pop(){
        if(empty())
            return;
        Node* temp=frontNode;
        frontNode=frontNode->next;
        if(frontNode==nullptr)
            rearNode=nullptr;
        delete temp;
        count--;
    }
    T& front(){
        return frontNode->data;
    }
};
template<class T>
class MyStack{
private:
    struct Node{
        T data;
        Node* next;
        Node(const T& value){
            data=value;
            next=nullptr;
        }
    };
    Node* topNode=nullptr;
    int count=0;
    void clear(){
        while(topNode!=nullptr){
            Node* temp=topNode;
            topNode=topNode->next;
            delete temp;
        }
        count=0;
    }
public:
    MyStack(){}
    MyStack(const MyStack& other){
        vector<T> values;
        Node* current=other.topNode;
        while(current!=nullptr){
            values.push_back(current->data);
            current=current->next;
        }
        for(int i=(int)values.size()-1;i>=0;i--)
            push(values[i]);
    }
    MyStack& operator=(const MyStack& other){
        if(this!=&other){
            clear();
            vector<T> values;
            Node* current=other.topNode;
            while(current!=nullptr){
                values.push_back(current->data);
                current=current->next;
            }
            for(int i=(int)values.size()-1;i>=0;i--)
                push(values[i]);
        }
        return *this;
    }
    ~MyStack(){
        clear();
    }
    bool empty() const{
        return topNode==nullptr;
    }
    int size() const{
        return count;
    }
    void push(const T& value){
        Node* temp=new Node(value);
        temp->next=topNode;
        topNode=temp;
        count++;
    }
    void pop(){
        if(empty())
            return;
        Node* temp=topNode;
        topNode=topNode->next;
        delete temp;
        count--;
    }
    T& top(){
        return topNode->data;
    }
};
class LeagueManager{
private:
    TeamList teams;
    MyQueue<Match> pending;
    MyStack<Team*> eliminated;
    vector<BracketNode> bracket;
    vector<Team*> allTeams;
    bool started=false,finished=false,finalRound=false;
    bool threeTeamSemi=false,placementRound=false;
    int nextMatchId=1,nextTeamId=1,roundNo=0;
    string currentStage;
    Team* byeTeam=nullptr;
    vector<Team*> roundWinners,semiLosers;
    Team *champion=nullptr,*runnerUp=nullptr;
    Team *thirdPlace=nullptr,*fourthPlace=nullptr;
    Match createMatch(Team* a,Team* b,MatchType type,string stage,int pair){
        Match m;
        m.id=nextMatchId++;
        m.a=a;
        m.b=b;
        m.type=type;
        m.stage=stage;
        m.pairIdx=pair;
        if(type!=BRONZE&&type!=PLACEMENT){
            BracketNode node;
            node.text="Match "+to_string(m.id)+": "+a->name+" vs "+b->name;
            node.left=a->bracketNode;
            node.right=b->bracketNode;
            bracket.push_back(node);
            m.nodeIdx=(int)bracket.size()-1;
            a->bracketNode=m.nodeIdx;
            b->bracketNode=m.nodeIdx;
        }
        pending.push(m);
        return m;
    }
    void printPending(){
        if(pending.empty()){
            cout<<"No pending matches.\n";
            return;
        }
        MyQueue<Match> temp=pending;
        while(!temp.empty()){
            Match m=temp.front();
            temp.pop();
            cout<<"Match "<<m.id<<": "<<m.a->name<<" vs "<<m.b->name<<" | "<<m.stage;
            if(m.replays>0)
                cout<<" | Replays: "<<m.replays;
            cout<<"\n";
        }
    }
    void startRound(vector<Team*> players){
        roundWinners.clear();
        byeTeam=nullptr;
        if(players.size()==2){
            finalRound=true;
            currentStage="Final";
        }else{
            finalRound=false;
            roundNo++;
            if(players.size()<=4)
                currentStage="Semifinal";
            else
                currentStage="Round "+to_string(roundNo);
            threeTeamSemi=(currentStage=="Semifinal"&&players.size()==3);
        }
        if(players.size()%2==1){
            int selected=-1;
            for(int i=0;i<(int)players.size();i++){
                if(!players[i]->hadBye){
                    selected=i;
                    break;
                }
            }
            if(selected==-1)
                selected=0;
            byeTeam=players[selected];
            byeTeam->hadBye=true;
            cout<<"\nBYE: "<<byeTeam->name<<" advances to the next round.\n";
            players.erase(players.begin()+selected);
        }
        roundWinners.assign(players.size()/2,nullptr);
        int left=0;
        int right=(int)players.size()-1;
        int pair=0;
        while(left<right){
            MatchType type=finalRound?FINAL_MATCH:NORMAL;
            createMatch(players[left],players[right],type,currentStage,pair);
            left++;
            right--;
            pair++;
        }
        if(finalRound&&semiLosers.size()==2)
            createMatch(semiLosers[0],semiLosers[1],BRONZE,"Bronze Match",0);
        cout<<"\n"<<currentStage<<" matches generated.\n";
        printPending();
    }
    void eliminate(Team* loser,string reason){
        if(loser==nullptr)
            return;
        loser->exitNote=reason;
        eliminated.push(loser);
    }
    void teamWon(Match m,Team* winner,Team* loser){
        if(m.type==PLACEMENT){
            runnerUp=winner;
            thirdPlace=loser;
            eliminate(winner,"Finished second place");
            eliminate(loser,"Finished third place");
            cout<<winner->name<<" secured second place.\n";
            cout<<loser->name<<" secured third place.\n";
            return;
        }
        if(m.type==BRONZE){
            thirdPlace=winner;
            fourthPlace=loser;
            eliminate(loser,"Finished fourth place");
            eliminate(winner,"Finished third place");
            cout<<winner->name<<" secured third place.\n";
            return;
        }
        if(m.nodeIdx!=-1)
            bracket[m.nodeIdx].text=m.stage+": "+winner->name+" defeated "+loser->name;
        if(m.type==FINAL_MATCH){
            champion=winner;
            runnerUp=loser;
            if(!threeTeamSemi)
                eliminate(loser,"Lost the final");
            cout<<"\nCHAMPION: "<<champion->name<<"\n";
        }else if(m.stage=="Semifinal")
            semiLosers.push_back(loser);
        else
            eliminate(loser,"Lost in "+m.stage);
        if(m.pairIdx>=0&&m.pairIdx<(int)roundWinners.size())
            roundWinners[m.pairIdx]=winner;
    }
    void replay(Match m){
        m.replays++;
        if(m.nodeIdx!=-1)
            bracket[m.nodeIdx].text="Match "+to_string(m.id)+": "+m.a->name+" vs "+m.b->name+" (Replay "+to_string(m.replays)+")";
        pending.push(m);
        cout<<"Match moved to the back of the queue.\n";
    }
    void finishStage(){
        if(placementRound){
            placementRound=false;
            finished=true;
            showFinalResults();
            return;
        }
        if(finalRound){
            if(threeTeamSemi&&semiLosers.size()==1){
                placementRound=true;
                finalRound=false;
                cout<<"\nFinal completed!";
                cout<<"\nGenerating second and third place match...\n";
                createMatch(runnerUp,semiLosers[0],PLACEMENT,"Second/Third Place Match",0);
                printPending();
                return;
            }
            if(semiLosers.size()==1){
                thirdPlace=semiLosers[0];
                eliminate(thirdPlace,"Finished third place");
            }
            finished=true;
            showFinalResults();
            return;
        }
        vector<Team*> nextRound=roundWinners;
        if(byeTeam!=nullptr)
            nextRound.push_back(byeTeam);
        cout<<"\nRound completed.\n";
        startRound(nextRound);
    }
    void printTree(int index,int depth){
        if(index==-1)
            return;
        for(int i=0;i<depth;i++)
            cout<<"   ";
        cout<<bracket[index].text<<"\n";
        printTree(bracket[index].left,depth+1);
        printTree(bracket[index].right,depth+1);
    }
    void printBracket(){
        if(bracket.empty()){
            cout<<"Bracket not generated yet.\n";
            return;
        }
        vector<bool> hasParent(bracket.size(),false);
        for(int i=0;i<(int)bracket.size();i++){
            if(bracket[i].left!=-1)
                hasParent[bracket[i].left]=true;
            if(bracket[i].right!=-1)
                hasParent[bracket[i].right]=true;
        }
        cout<<"\n========== TOURNAMENT BRACKET ==========\n";
        for(int i=0;i<(int)bracket.size();i++)
            if(!hasParent[i])
                printTree(i,0);
    }
    void showFinalResults(){
        cout<<"\n========== FINAL RESULTS ==========\n";
        if(champion!=nullptr)
            cout<<"1st Place: "<<champion->name<<"\n";
        if(runnerUp!=nullptr)
            cout<<"2nd Place: "<<runnerUp->name<<"\n";
        if(thirdPlace!=nullptr)
            cout<<"3rd Place: "<<thirdPlace->name<<"\n";
        if(fourthPlace!=nullptr)
            cout<<"4th Place: "<<fourthPlace->name<<"\n";
        cout<<"\n========== ELIMINATED TEAMS ==========\n";
        if(eliminated.empty())
            cout<<"No eliminated teams.\n";
        else{
            MyStack<Team*> temp=eliminated;
            while(!temp.empty()){
                Team* t=temp.top();
                temp.pop();
                cout<<t->name<<" | "<<t->exitNote<<"\n";
            }
        }
        printBracket();
        cout<<"\nTournament completed successfully.\n";
    }
public:
    void registerTeam(){
        if(started){
            cout<<"Registration closed. League already started.\n";
            return;
        }
        string name,captain;
        cout<<"Enter team name: ";
        if(!getline(cin,name))
            return;
        if(isBlank(name)){
            cout<<"Invalid team name.\n";
            return;
        }
        if(teams.exists(name)){
            cout<<"Team already exists.\n";
            return;
        }
        cout<<"Enter captain name: ";
        if(!getline(cin,captain))
            return;
        if(isBlank(captain)){
            cout<<"Invalid captain name.\n";
            return;
        }
        teams.add(nextTeamId++,name,captain);
        cout<<"Team registered successfully.\n";
    }
    void viewTeams(){
        cout<<"\n========== REGISTERED TEAMS ==========\n";
        teams.print();
    }
    void startLeague(){
        if(started){
            cout<<"League has already started.\n";
            return;
        }
        if(teams.count<2){
            cout<<"Register at least two teams first.\n";
            return;
        }
        string confirm;
        cout<<"Start the knockout tournament? (y/n): ";
        if(!getline(cin,confirm))
            return;
        if(confirm!="y"&&confirm!="Y"){
            cout<<"Tournament start cancelled.\n";
            return;
        }
        started=true;
        allTeams=teams.toVector();
        for(int i=0;i<(int)allTeams.size();i++){
            BracketNode node;
            node.text=allTeams[i]->name;
            bracket.push_back(node);
            allTeams[i]->bracketNode=(int)bracket.size()-1;
        }
        cout<<"\nTournament started!\n";
        cout<<"Teams paired using first-versus-last order.\n";
        startRound(allTeams);
    }
    void showPending(){
        if(!started){
            cout<<"Start the league first.\n";
            return;
        }
        cout<<"\n========== UPCOMING MATCHES ==========\n";
        printPending();
    }
    void enterResult(){
        if(!started||finished){
            cout<<"No active tournament matches.\n";
            return;
        }
        if(pending.empty()){
            cout<<"No pending matches.\n";
            return;
        }
        Match m=pending.front();
        cout<<"\n========== ENTER MATCH RESULT ==========\n";
        cout<<"Match "<<m.id<<"\n";
        cout<<m.a->name<<" vs "<<m.b->name<<"\n";
        cout<<"\n1. "<<m.a->name<<" wins\n";
        cout<<"2. "<<m.b->name<<" wins\n";
        cout<<"3. Draw\n";
        cout<<"4. Cancel match and replay\n";
        cout<<"0. Back\n";
        string input;
        cout<<"Enter result: ";
        if(!getline(cin,input))
            return;
        if(input.size()!=1||input[0]<'0'||input[0]>'4'){
            cout<<"Invalid choice.\n";
            return;
        }
        char option=input[0];
        if(option=='0')
            return;
        pending.pop();
        if(option=='1')
            teamWon(m,m.a,m.b);
        else if(option=='2')
            teamWon(m,m.b,m.a);
        else
            replay(m);
        if(pending.empty())
            finishStage();
    }
    void showStatus(){
        cout<<"\n========== LEAGUE STATUS ==========\n";
        if(!started)
            cout<<"Status: Not started\n";
        else if(finished)
            cout<<"Status: Completed\n";
        else
            cout<<"Status: In progress\n";
        cout<<"Registered teams: "<<teams.count<<"\n";
        cout<<"Pending matches: "<<pending.size()<<"\n";
        if(started)
            printBracket();
    }
};
void printMenu(){
    cout<<"\n========== SPORTS LEAGUE MANAGEMENT ==========\n";
    cout<<"1. Register a team\n";
    cout<<"2. View registered teams\n";
    cout<<"3. Start league\n";
    cout<<"4. Upcoming matches\n";
    cout<<"5. Enter match result\n";
    cout<<"6. League status and bracket\n";
    cout<<"0. Exit\n";
}
int main(){
    LeagueManager league;
    string input;
    while(true){
        printMenu();
        cout<<"Enter your choice: ";
        if(!getline(cin,input))
            break;
        if(input.size()!=1||input[0]<'0'||input[0]>'6'){
            cout<<"Please enter a valid number between 0 and 6.\n";
            continue;
        }
        switch(input[0]){
            case '1':
                league.registerTeam();
                break;
            case '2':
                league.viewTeams();
                break;
            case '3':
                league.startLeague();
                break;
            case '4':
                league.showPending();
                break;
            case '5':
                league.enterResult();
                break;
            case '6':
                league.showStatus();
                break;
            case '0':
                cout<<"Exiting Sports League Management System.\n";
                return 0;
        }
    }
    return 0;
}
