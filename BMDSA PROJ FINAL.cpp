#include <bits/stdc++.h>
using namespace std;
const int SLOTS_PER_DAY = 6;
const char* const SLOT_TIME[SLOTS_PER_DAY] = {"08:00", "10:00", "12:00", "14:00", "16:00", "18:00"};
const int MAX_DAY   = 365;
const int MAX_TEAMS = 64;

int    makeKey(int day, int slot) { return day * SLOTS_PER_DAY + slot; }
int    dayOf(int key)             { return key / SLOTS_PER_DAY; }
int    slotOf(int key)            { return key % SLOTS_PER_DAY; }
string slotText(int key)          { return "Day " + to_string(dayOf(key)) + ", " + SLOT_TIME[slotOf(key)]; }

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

string toLower(string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

string readLine(const string& prompt, size_t maxLen = 30) {
    while (true) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) { cout << "\nInput ended. Bye!\n"; exit(0); }
        s = trim(s);
        if (s.empty())        { cout << "  Input cannot be empty.\n"; continue; }
        if (s.size() > maxLen){ cout << "  Too long (max " << maxLen << " characters).\n"; continue; }
        return s;
    }
}

int readInt(const string& prompt, int lo, int hi) {
    while (true) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) { cout << "\nInput ended. Bye!\n"; exit(0); }
        s = trim(s);
        try {
            size_t pos = 0;
            int v = stoi(s, &pos);
            if (pos == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {}
        cout << "  Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

struct Team {
    int    id = 0;
    string name, captain;
    bool   hadBye = false;
    int    bracketNode = -1;
    string exitNote;
    Team*  next = nullptr;
};

class TeamList {
    Team* head = nullptr;
    Team* tail = nullptr;
    int   count = 0;
public:
    TeamList() {}
    TeamList(const TeamList&) = delete;
    TeamList& operator=(const TeamList&) = delete;
    ~TeamList() { while (head) { Team* t = head; head = head->next; delete t; } }

    int size() const { return count; }

    bool exists(const string& name) const {
        string key = toLower(name);
        for (Team* t = head; t; t = t->next)
            if (toLower(t->name) == key) return true;
        return false;
    }

    Team* add(const string& name, const string& captain) {
        Team* t = new Team();
        t->id = count++;
        t->name = name;
        t->captain = captain;
        if (!head) head = tail = t;
        else { tail->next = t; tail = t; }
        return t;
    }

    vector<Team*> toVector() const {
        vector<Team*> v;
        for (Team* t = head; t; t = t->next) v.push_back(t);
        return v;
    }

    void print() const {
        if (!head) { cout << "  (no teams registered yet)\n"; return; }
        int i = 1;
        for (Team* t = head; t; t = t->next, i++)
            cout << "  " << i << ". " << t->name << "  (captain: " << t->captain << ")\n";
    }
};

struct Booking {
    int     key;
    char    type;
    string  text;
    Booking *left, *right;
    Booking(int k, char t, const string& s) : key(k), type(t), text(s), left(nullptr), right(nullptr) {}
};

class CourtBST {
    Booking* root = nullptr;

    Booking* insert(Booking* node, int key, char type, const string& text, bool& ok) {
        if (!node) { ok = true; return new Booking(key, type, text); }
        if (key < node->key)      node->left  = insert(node->left,  key, type, text, ok);
        else if (key > node->key) node->right = insert(node->right, key, type, text, ok);

        return node;
    }

    Booking* findMin(Booking* node) { while (node->left) node = node->left; return node; }

    Booking* remove(Booking* node, int key, bool& removed) {
        if (!node) return nullptr;
        if (key < node->key)      node->left  = remove(node->left,  key, removed);
        else if (key > node->key) node->right = remove(node->right, key, removed);
        else {
            removed = true;
            if (!node->left)  { Booking* r = node->right; delete node; return r; }
            if (!node->right) { Booking* l = node->left;  delete node; return l; }
            Booking* m = findMin(node->right);
            int mk = m->key;
            node->key = m->key; node->type = m->type; node->text = m->text;
            bool dummy = false;
            node->right = remove(node->right, mk, dummy);
        }
        return node;
    }

    void inorder(Booking* node, vector<const Booking*>& out) const {
        if (!node) return;
        inorder(node->left, out);
        out.push_back(node);
        inorder(node->right, out);
    }

    void destroy(Booking* node) {
        if (!node) return;
        destroy(node->left); destroy(node->right); delete node;
    }

public:
    CourtBST() {}
    CourtBST(const CourtBST&) = delete;
    CourtBST& operator=(const CourtBST&) = delete;
    ~CourtBST() { destroy(root); }

    bool book(int key, char type, const string& text) {
        bool ok = false;
        root = insert(root, key, type, text, ok);
        return ok;
    }

    const Booking* find(int key) const {
        Booking* cur = root;
        while (cur) {
            if (key == cur->key) return cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return nullptr;
    }

    bool cancel(int key) { bool removed = false; root = remove(root, key, removed); return removed; }

    vector<const Booking*> all() const { vector<const Booking*> v; inorder(root, v); return v; }

    int nextFree(int fromKey) const {
        int k = fromKey;
        while (find(k)) k++;
        return k;
    }
};

enum MatchType { NORMAL, BRONZE, FINAL_MATCH };

struct Match {
    int       id = 0;
    Team*     a = nullptr;
    Team*     b = nullptr;
    MatchType type = NORMAL;
    string    stage;
    int       slotKey = 0;
    int       pairIdx = -1;
    int       nodeIdx = -1;
    int       replays = 0;
};

struct BracketNode {
    string text;
    int left = -1, right = -1;
};


class LeagueManager {
    TeamList teams;
    CourtBST court;
    queue<Match> pending;
    stack<Team*> eliminated;
    vector<BracketNode> bracket;
    vector<vector<int>> beats;
    vector<Team*> allTeams;

    bool started = false, finished = false;
    int  currentDay = 1, nextMatchId = 1, lastLeagueKey = 0, roundNo = 0;


    string stageName;
    bool   semiRound = false, finalRound = false;
    Team*  byeTeam = nullptr;
    vector<Team*> winnersByPair;
    vector<Team*> semiLosers;
    Team *gold = nullptr, *silver = nullptr, *bronze = nullptr, *fourth = nullptr;


    string matchLabel(const Match& m, const string& result) const {
        return "Match #" + to_string(m.id) + " [" + m.stage + "]: " + m.a->name + " vs " + m.b->name + " => " + result;
    }


    void scheduleMatch(Match& m) {
        int from = max(currentDay * SLOTS_PER_DAY, lastLeagueKey + 1);
        m.slotKey = court.nextFree(from);
        lastLeagueKey = m.slotKey;
        string text = "MATCH #" + to_string(m.id) + ": " + m.a->name + " vs " + m.b->name + " [" + m.stage + "]";
        if (m.replays > 0) text += " (replay " + to_string(m.replays) + ")";
        court.book(m.slotKey, 'M', text);
    }

    void createMatch(Team* a, Team* b, MatchType type, int pairIdx) {
        Match m;
        m.id = nextMatchId++;
        m.a = a; m.b = b; m.type = type; m.pairIdx = pairIdx;
        m.stage = (type == BRONZE) ? "BRONZE MATCH" : stageName;
        if (type != BRONZE) {
            BracketNode n;
            n.left = a->bracketNode;
            n.right = b->bracketNode;
            n.text = matchLabel(m, "pending");
            bracket.push_back(n);
            m.nodeIdx = (int)bracket.size() - 1;
        }
        scheduleMatch(m);
        pending.push(m);
    }

    void printPending() const {
        if (pending.empty()) { cout << "  No pending matches.\n"; return; }
        queue<Match> copy = pending;
        while (!copy.empty()) {
            const Match& m = copy.front();
            cout << "  Match #" << m.id << " [" << m.stage << "]  " << m.a->name << " vs " << m.b->name
                 << "  ->  " << slotText(m.slotKey);
            if (m.replays > 0) cout << "  (replay " << m.replays << ")";
            cout << "\n";
            copy.pop();
        }
    }


    void startRound(vector<Team*> players) {
        roundNo++;
        int n = (int)players.size();
        finalRound = (n == 2);
        semiRound  = (n == 3 || n == 4);
        if (finalRound)      stageName = "FINAL";
        else if (semiRound)  stageName = "SEMI-FINAL";
        else                 stageName = "ROUND " + to_string(roundNo);


        byeTeam = nullptr;
        if (n % 2 == 1) {
            int idx = n - 1;
            for (int i = n - 1; i >= 0; i--)
                if (!players[i]->hadBye) { idx = i; break; }
            byeTeam = players[idx];
            byeTeam->hadBye = true;
            players.erase(players.begin() + idx);
        }
        winnersByPair.assign(players.size() / 2, nullptr);

        cout << "\n=============== " << stageName << " ===============\n";
        if (byeTeam) cout << byeTeam->name << " gets a BYE and moves to the next stage.\n";


        if (finalRound && semiLosers.size() == 2) createMatch(semiLosers[0], semiLosers[1], BRONZE, -1);

        for (size_t i = 0; i + 1 < players.size(); i += 2)
            createMatch(players[i], players[i + 1], finalRound ? FINAL_MATCH : NORMAL, (int)(i / 2));

        cout << "Scheduled matches:\n";
        printPending();
    }


    void finishStage() {
        if (finalRound) { finished = true; showFinalResults(); return; }
        vector<Team*> next;
        if (byeTeam) next.push_back(byeTeam);
        for (size_t i = 0; i < winnersByPair.size(); i++) next.push_back(winnersByPair[i]);
        startRound(next);
    }


    void eliminate(Team* loser, const Match& m, Team* winner, const string& headline) {
        loser->exitNote = "lost Match #" + to_string(m.id) + " [" + m.stage + "] against " + winner->name;
        eliminated.push(loser);
        cout << "*** " << loser->name << " " << headline << " - " << loser->exitNote << " ***\n";
    }

    void teamWon(const Match& m, Team* winner, Team* loser) {
        beats[winner->id].push_back(loser->id);
        cout << "\nMatch #" << m.id << " [" << m.stage << "]: " << winner->name << " beat " << loser->name << "\n";

        if (m.nodeIdx >= 0) {
            bracket[m.nodeIdx].text = matchLabel(m, winner->name + " won");
            winner->bracketNode = m.nodeIdx;
        }

        if (m.type == BRONZE) {
            bronze = winner; fourth = loser;
            cout << winner->name << " wins the BRONZE medal!\n";
            eliminate(loser, m, winner, "finishes 4th");
        } else if (m.type == FINAL_MATCH) {
            gold = winner; silver = loser;
            cout << winner->name << " wins the FINAL and takes GOLD!\n";
        } else {
            winnersByPair[m.pairIdx] = winner;
            if (semiRound) {
                semiLosers.push_back(loser);
                if (winnersByPair.size() == 1) {
                    bronze = loser;
                    cout << loser->name << " lost the semi-final and gets the BRONZE medal.\n";
                } else {
                    cout << loser->name << " lost the semi-final and will play the BRONZE match.\n";
                }
            } else {
                eliminate(loser, m, winner, "is ELIMINATED");
            }
        }
    }


    void replay(Match m, bool cancelled) {
        if (cancelled) court.cancel(m.slotKey);
        m.replays++;
        scheduleMatch(m);
        pending.push(m);
        if (m.nodeIdx >= 0)
            bracket[m.nodeIdx].text = matchLabel(m, cancelled ? "CANCELLED - replay scheduled" : "DRAW - replay scheduled");
        cout << "\nMatch #" << m.id << (cancelled ? " was CANCELLED." : " ended in a DRAW.")
             << " Same teams will play again: " << slotText(m.slotKey) << "\n";
    }


    void printTree(int idx, int depth) const {
        cout << string(depth * 4, ' ') << (depth ? "+-- " : "") << bracket[idx].text << "\n";
        if (bracket[idx].left  >= 0) printTree(bracket[idx].left,  depth + 1);
        if (bracket[idx].right >= 0) printTree(bracket[idx].right, depth + 1);
    }

    void printBracket() const {
        vector<bool> hasParent(bracket.size(), false);
        for (size_t i = 0; i < bracket.size(); i++) {
            if (bracket[i].left  >= 0) hasParent[bracket[i].left]  = true;
            if (bracket[i].right >= 0) hasParent[bracket[i].right] = true;
        }
        for (size_t i = 0; i < bracket.size(); i++)
            if (!hasParent[i]) printTree((int)i, 0);
    }

    void printVictoryChain() const {
        vector<int> dist(allTeams.size(), -1);
        queue<int> q;
        dist[gold->id] = 0;
        q.push(gold->id);
        int maxD = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (size_t i = 0; i < beats[u].size(); i++) {
                int v = beats[u][i];
                if (dist[v] == -1) { dist[v] = dist[u] + 1; maxD = max(maxD, dist[v]); q.push(v); }
            }
        }
        cout << "\nVictory chain (BFS from the champion):\n";
        for (int d = 0; d <= maxD; d++) {
            cout << "  Level " << d << ": ";
            bool first = true;
            for (size_t i = 0; i < allTeams.size(); i++)
                if (dist[i] == d) { cout << (first ? "" : ", ") << allTeams[i]->name; first = false; }
            cout << "\n";
        }
        cout << "\nWho beat whom:\n";
        for (size_t i = 0; i < allTeams.size(); i++) {
            if (beats[i].empty()) continue;
            cout << "  " << allTeams[i]->name << " beat: ";
            for (size_t j = 0; j < beats[i].size(); j++)
                cout << (j ? ", " : "") << allTeams[beats[i][j]]->name;
            cout << "\n";
        }
    }

    void showFinalResults() const {
        cout << "\n################ FINAL RESULTS ################\n";
        cout << "  GOLD   : " << gold->name   << "\n";
        cout << "  SILVER : " << silver->name << "\n";
        if (bronze) cout << "  BRONZE : " << bronze->name << "\n";
        else        cout << "  BRONZE : not awarded (needs at least 3 teams)\n";
        if (fourth) cout << "  4th    : " << fourth->name << "\n";

        stack<Team*> copy = eliminated;
        bool header = false;
        while (!copy.empty()) {
            Team* t = copy.top(); copy.pop();
            if (t == fourth) continue;
            if (!header) { cout << "\nOther teams (latest exit first):\n"; header = true; }
            cout << "  " << t->name << " - " << t->exitNote << "\n";
        }
        printVictoryChain();
        cout << "\nBracket:\n";
        printBracket();
        cout << "###############################################\n";
    }

public:

    int today() const { return currentDay; }

    void registerTeam() {
        if (started)                    { cout << "Registration is closed - the league has started.\n"; return; }
        if (teams.size() >= MAX_TEAMS)  { cout << "Maximum of " << MAX_TEAMS << " teams reached.\n"; return; }
        string name = readLine("Team name: ");
        if (teams.exists(name))         { cout << "  A team named '" << name << "' already exists.\n"; return; }
        string cap = readLine("Captain name: ");
        teams.add(name, cap);
        cout << "Team '" << name << "' registered. Total teams: " << teams.size() << "\n";
    }

    void viewTeams() const {
        cout << "\nRegistered teams (" << teams.size() << "):\n";
        teams.print();
    }

    void startLeague() {
        if (started) { cout << "League has already started.\n"; return; }
        int n = teams.size();
        if (n < 2)   { cout << "Need at least 2 teams to start a league (now: " << n << ").\n"; return; }
        int yes = readInt("Registration will close. Start the league with " + to_string(n) + " teams? (1 = yes, 0 = no): ", 0, 1);
        if (!yes) return;

        started = true;
        allTeams = teams.toVector();
        beats.assign(n, vector<int>());
        for (size_t i = 0; i < allTeams.size(); i++) {
            BracketNode leaf;
            leaf.text = allTeams[i]->name;
            bracket.push_back(leaf);
            allTeams[i]->bracketNode = (int)bracket.size() - 1;
        }

        int rounds = 0;
        for (int k = n; k > 1; k = (k + 1) / 2) rounds++;
        cout << "\nLeague created: " << n << " teams, " << rounds << " round(s), "
             << (n - 1 + (n >= 4 ? 1 : 0)) << " match(es) in total"
             << (n >= 4 ? " (including the bronze match)" : "") << ".\n";
        cout << "Later rounds are scheduled automatically as winners are decided.\n";

        vector<Team*> players = allTeams;
        mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
        shuffle(players.begin(), players.end(), rng);
        startRound(players);
    }

    void showPending() const {
        if (!started)  { cout << "League not started yet.\n"; return; }
        if (finished)  { cout << "League is over.\n"; return; }
        cout << "\nCurrent stage: " << stageName << "\nUpcoming matches (in order):\n";
        printPending();
    }

    void enterResult() {
        if (!started) { cout << "League not started yet.\n"; return; }
        if (finished) { cout << "League is over. Nothing left to play.\n"; return; }

        Match m = pending.front();
        cout << "\nNext match: #" << m.id << " [" << m.stage << "]  " << m.a->name << " vs " << m.b->name
             << "  (" << slotText(m.slotKey) << ")\n";
        cout << "  1. " << m.a->name << " won\n  2. " << m.b->name << " won\n  3. Draw\n  4. Cancelled\n  0. Back\n";
        int c = readInt("Result: ", 0, 4);
        if (c == 0) return;

        pending.pop();
        currentDay = max(currentDay, dayOf(m.slotKey));
        if (c == 1)      teamWon(m, m.a, m.b);
        else if (c == 2) teamWon(m, m.b, m.a);
        else             replay(m, c == 4);

        if (pending.empty()) finishStage();
    }

    void showStatus() const {
        if (!started) { cout << "League not started yet.\n"; return; }
        cout << "\nStage: " << (finished ? "LEAGUE FINISHED" : stageName) << "\n";
        if (!finished) { cout << "Pending matches:\n"; printPending(); }

        stack<Team*> copy = eliminated;
        cout << "\nOut of the competition (latest first):\n";
        if (copy.empty()) cout << "  (nobody yet)\n";
        while (!copy.empty()) { cout << "  " << copy.top()->name << " - " << copy.top()->exitNote << "\n"; copy.pop(); }

        cout << "\nBracket so far:\n";
        printBracket();
        if (finished) showFinalResults();
    }


    void showDay(int day) const {
        cout << "\nCourt on Day " << day << ":\n";
        for (int s = 0; s < SLOTS_PER_DAY; s++) {
            const Booking* b = court.find(makeKey(day, s));
            cout << "  Slot " << (s + 1) << "  " << SLOT_TIME[s] << "  : " << (b ? b->text : "FREE") << "\n";
        }
    }

    void checkAvailability() const {
        int day = readInt("Day number (" + to_string(currentDay) + "-" + to_string(MAX_DAY) + "): ", currentDay, MAX_DAY);
        showDay(day);
    }

    void bookPractice() {
        string who = readLine("Student / group name: ");
        int day = readInt("Day number (" + to_string(currentDay) + "-" + to_string(MAX_DAY) + "): ", currentDay, MAX_DAY);
        showDay(day);
        int slot = readInt("Slot number (1-" + to_string(SLOTS_PER_DAY) + "): ", 1, SLOTS_PER_DAY);
        int key = makeKey(day, slot - 1);
        const Booking* b = court.find(key);
        if (b) {
            cout << "Sorry, that slot is taken (" << b->text << ").\n";
            cout << "Next free slot: " << slotText(court.nextFree(key)) << "\n";
            return;
        }
        court.book(key, 'P', "PRACTICE: " + who);
        cout << "Practice booked for " << who << " at " << slotText(key) << ".\n";
    }

    void cancelPractice() {
        int day = readInt("Day number (" + to_string(currentDay) + "-" + to_string(MAX_DAY) + "): ", currentDay, MAX_DAY);
        showDay(day);
        int slot = readInt("Slot number to cancel (1-" + to_string(SLOTS_PER_DAY) + "): ", 1, SLOTS_PER_DAY);
        int key = makeKey(day, slot - 1);
        const Booking* b = court.find(key);
        if (!b)                 { cout << "That slot has no booking.\n"; return; }
        if (b->type != 'P')     { cout << "That slot is a league match - it cannot be cancelled here.\n"; return; }
        court.cancel(key);
        cout << "Practice booking cancelled. Slot is free again.\n";
    }

    void showCourtSchedule() const {
        vector<const Booking*> all = court.all();
        cout << "\nFull court schedule:\n";
        if (all.empty()) { cout << "  (no bookings)\n"; return; }
        for (size_t i = 0; i < all.size(); i++)
            cout << "  " << slotText(all[i]->key) << "  ->  " << all[i]->text << "\n";
    }

    void advanceDay() {
        if (currentDay >= MAX_DAY) { cout << "Reached the last day of the calendar.\n"; return; }
        currentDay++;
        cout << "It is now Day " << currentDay << ".\n";
    }
};


void printMenu(int day) {
    cout << "\n=========== SPORTS LEAGUE MANAGER  (Today: Day " << day << ") ===========\n"
         << "  1. Register a team            7. Court: check a day\n"
         << "  2. View registered teams      8. Court: book practice slot\n"
         << "  3. Start league               9. Court: cancel practice slot\n"
         << "  4. Upcoming matches          10. Court: full schedule\n"
         << "  5. Enter match result        11. Advance to next day\n"
         << "  6. League status / bracket    0. Exit\n";
}

int main() {
    LeagueManager league;
    cout << "Welcome to the Automated Sports League Manager!\n";
    while (true) {
        printMenu(league.today());
        int choice = readInt("Choose an option: ", 0, 11);
        switch (choice) {
            case 1:  league.registerTeam();      break;
            case 2:  league.viewTeams();         break;
            case 3:  league.startLeague();       break;
            case 4:  league.showPending();       break;
            case 5:  league.enterResult();       break;
            case 6:  league.showStatus();        break;
            case 7:  league.checkAvailability(); break;
            case 8:  league.bookPractice();      break;
            case 9:  league.cancelPractice();    break;
            case 10: league.showCourtSchedule(); break;
            case 11: league.advanceDay();        break;
            case 0:  cout << "Goodbye!\n";       return 0;
        }
    }
}