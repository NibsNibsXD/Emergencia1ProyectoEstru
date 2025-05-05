#ifndef CHATCORE_H
#define CHATCORE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <regex>
#include <vector>            // para contactsSorted(...)
#include <unordered_map>

/* =========================================================
 *                 Estructuras genéricas
 * =========================================================*/
template<typename T>
struct Node {
    T data;
    Node* next;
    explicit Node(T d): data(d), next(nullptr) {}
};

template<typename T>
class LinkedList {
    Node<T>* h{};
public:
    ~LinkedList() { while (h) { auto* t = h; h = h->next; delete t; } }

    bool empty() const { return h == nullptr; }

    void push_back(const T& d) {
        auto* n = new Node<T>(d);
        if (!h) { h = n; return; }
        auto* c = h; while (c->next) c = c->next;
        c->next = n;
    }
    bool contains(const T& v) const {
        for (auto* c = h; c; c = c->next)
            if (c->data == v) return true;
        return false;
    }
    void bubbleSort() {
        if (!h) return;
        bool sw;
        do {
            sw = false;
            for (auto* c = h; c->next; c = c->next)
                if (c->data > c->next->data) {
                    std::swap(c->data, c->next->data); sw = true;
                }
        } while (sw);
    }
    void forEach(void (*fn)(const T&)) const {
        for (auto* c = h; c; c = c->next) fn(c->data);
    }
};

template<typename T>
class Stack {
    Node<T>* top{};
public:
    ~Stack() { while (top) pop(); }
    bool empty() const { return !top; }
    void push(const T& d) { auto* n = new Node<T>(d); n->next = top; top = n; }
    T pop() { auto* t = top; top = t->next; T d = t->data; delete t; return d; }
};

template<typename T>
class Queue {
    Node<T>* f{}; Node<T>* b{};
public:
    ~Queue() { while (f) dequeue(); }
    bool empty() const { return !f; }
    void enqueue(const T& d) {
        auto* n = new Node<T>(d);
        if (!f) { f = b = n; } else { b->next = n; b = n; }
    }
    T dequeue() {
        auto* t = f; f = f->next; if (!f) b = nullptr;
        T d = t->data; delete t; return d;
    }
};

/* =========================================================
 *                       Mensajes
 * =========================================================*/
enum class MsgType { TEXT, STICKER };

template<typename P>
struct Message {
    std::string from, to;
    time_t      ts{};
    P           content{};
    MsgType     type{MsgType::TEXT};

    std::string serialize() const {
        std::ostringstream o;
        o << from << '|' << to << '|' << ts << '|' << int(type) << '|' << content;
        return o.str();
    }
    static Message deserialize(const std::string& l) {
        std::istringstream i(l); Message m; std::string t;
        getline(i, m.from, '|'); getline(i, m.to, '|');
        getline(i, t, '|'); m.ts   = std::stoll(t);
        getline(i, t, '|'); m.type = MsgType(std::stoi(t));
        getline(i, m.content);
        return m;
    }
};

/* =========================================================
 *                         Util
 * =========================================================*/
inline std::string timestampToStr(time_t ts) {
    char b[20]; std::strftime(b, 20, "%Y-%m-%d %H:%M", std::localtime(&ts));
    return b;
}
inline bool isValidEmail(const std::string& e) {
    return std::regex_match(e, std::regex(R"(^.+@.+\..+$)"));
}

/* =========================================================
 *                    Modelo usuario
 * =========================================================*/
struct User {
    std::string username, fullname, email, password, avatar;
    std::string secQ, secA;
    int  age{};
    LinkedList<std::string> contacts;

    std::string serializeBasic() const {
        std::ostringstream o;
        o << username << ',' << fullname << ',' << email << ',' << password << ','
          << age << ',' << avatar << ',' << secQ << ',' << secA;
        return o.str();
    }
    static User deserializeBasic(const std::string& l) {
        std::istringstream i(l);  User u;  std::string t;
        getline(i, u.username, ','); getline(i, u.fullname, ',');
        getline(i, u.email,   ','); getline(i, u.password, ',');
        getline(i, t, ',');  u.age = std::stoi(t);
        getline(i, u.avatar, ','); getline(i, u.secQ, ','); getline(i, u.secA, ',');
        return u;
    }
};

/* =========================================================
 *            Criterios de orden para contactos
 * =========================================================*/
enum class ContactSort {
    Alphabetical,
    LastMessageDesc,
    ChatLengthDesc
};

/* =========================================================
 *                       ChatCore
 * =========================================================*/
class ChatCore
{
    const std::string USERS_FILE = "users.txt";
    User current;
    bool logged = false;

    /* ====== persistencia interna ====== */
    bool loadUser(const std::string&, User&);
    void saveContact(const std::string&, const std::string&);
    void loadContacts(User&);
    std::string contactsFile(const std::string&);
    std::string historyFile(const std::string&, const std::string&) const;

    template<typename P>
    void appendMessageToHistory(const Message<P>& m) {
        std::ofstream(historyFile(m.from, m.to), std::ios::app) << m.serialize() << '\n';
    }

    /* ====== “no leídos” en memoria ====== */
    std::unordered_map<std::string, Queue<Message<std::string>>> pending;

    /* registros “visto” por contacto */
    std::unordered_map<std::string, time_t> seen;
    std::string seenFile() const { return current.username + "_seen.txt"; }
    void loadSeen();
    void saveSeen() const;

    /* ------ auxiliares para orden -------- */
    time_t lastMessageTs(const std::string& peer) const;
    int    chatLength   (const std::string& peer) const;

    /* ------------- modo consola ---------- */
    void printHistory(const std::string&, const std::string&);
    static void printer(const std::string&);
    void addContact();  void listContacts(); void chatMenu();
    void userMenu();    void login();        void registerUser();
    void run();

public:
    ChatCore() = default;

    /* ----------- autenticación ----------- */
    bool userExists(const std::string&);
    void saveUser(const User&);
    bool loginUser(const std::string&, const std::string&);
    bool loggedIn() const { return logged; }
    void logout() { logged = false; }

    /* --------- operaciones de perfil ------ */
    bool updateAvatar  (const std::string& path);
    bool updatePassword(const std::string& oldP, const std::string& newP);
    bool deleteCurrentUser();
    bool resetPassword(const std::string& user,
                       const std::string& answer,
                       const std::string& newPass);

    /* -------------- contactos ------------- */
    void persistContact(const std::string& c) { saveContact(current.username, c); }
    std::vector<std::string> contactsSorted(ContactSort how) const;

    /* --------------- mensajes ------------- */
    template<typename P>
    void saveMessage(const Message<P>& m) { appendMessageToHistory(m); }
    void removeLastFromHistory(const std::string&, const std::string&);

    /* ------------- no‑leídos -------------- */
    void  storeUnread(const Message<std::string>& m){ pending[m.from].enqueue(m); }
    Queue<Message<std::string>>      takeUnread(const std::string& from);
    int   unreadCount(const std::string& from) const;
    void  markSeen(const std::string& peer);

    /* -------- acceso a usuario ------------ */
    User&       currentUser()       { return current; }
    const User& currentUser() const { return current; }
};

#endif // CHATCORE_H
