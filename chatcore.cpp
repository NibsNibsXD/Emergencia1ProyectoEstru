#include "chatcore.h"
#include <cstdio>            // std::remove
#include <iterator>          // std::istreambuf_iterator
#include <algorithm>         // std::count
#include <vector>

/* =========================================================
 *  helpers internos: selection‑sort “casero” sobre std::vector
 * =========================================================*/
namespace {
template<typename Vec, typename Cmp>
void selectionSort(Vec& v, Cmp cmp)
{
    const size_t n = v.size();
    for (size_t i = 0; i + 1 < n; ++i) {
        size_t best = i;
        for (size_t j = i + 1; j < n; ++j)
            if (cmp(v[j], v[best]))
                best = j;
        if (best != i) std::swap(v[i], v[best]);
    }
}
} // namespace

/* =========================================================
 *              SECCIÓN 1 – funciones helper
 * =========================================================*/
time_t ChatCore::lastMessageTs(const std::string& peer) const
{
    std::ifstream f(historyFile(current.username, peer));
    std::string ln, last;
    while (std::getline(f, ln)) last = ln;
    if (last.empty()) return 0;
    return Message<std::string>::deserialize(last).ts;
}

int ChatCore::chatLength(const std::string& peer) const
{
    std::ifstream f(historyFile(current.username, peer));
    return std::count(std::istreambuf_iterator<char>(f),
                      std::istreambuf_iterator<char>(), '\n');
}

/* =========================================================
 *      Contactos ordenados  (selection‑sort casero)
 * =========================================================*/
static std::vector<std::string>* gVec = nullptr;   // ——— solo visible en este archivo
static void pushToVector(const std::string& s)     //   función compatible con forEach
{
    if(gVec) gVec->push_back(s);
}

std::vector<std::string>
ChatCore::contactsSorted(ContactSort how) const
{
    /* ---- 1) copiar lista enlazada a vector ---------------- */
    std::vector<std::string> v;
    gVec = &v;                  // establecer destino temporal
    current.contacts.forEach(pushToVector);
    gVec = nullptr;             // limpiar

    /* ---- 2) comparadores --------------------------------- */
    auto cmpAlpha = [](const std::string& a, const std::string& b){ return a < b; };

    auto cmpLast  = [this](const std::string& a, const std::string& b){
        return lastMessageTs(a) > lastMessageTs(b);   // más reciente primero
    };

    auto cmpLen   = [this](const std::string& a, const std::string& b){
        int la = chatLength(a), lb = chatLength(b);
        return (la == lb) ? a < b : la > lb;          // empate → alfabético
    };

    /* ---- 3) ordenar con selectionSort -------------------- */
    switch (how) {
    case ContactSort::Alphabetical:     selectionSort(v, cmpAlpha); break;
    case ContactSort::LastMessageDesc:  selectionSort(v, cmpLast ); break;
    case ContactSort::ChatLengthDesc:   selectionSort(v, cmpLen );  break;
    }
    return v;
}

/* =========================================================
 *      (resto de implementación sin cambios)
 * =========================================================*/

bool ChatCore::userExists(const std::string& uname)
{
    std::ifstream f(USERS_FILE);
    std::string line;
    while (std::getline(f, line))
        if (User::deserializeBasic(line).username == uname) return true;
    return false;
}

bool ChatCore::loadUser(const std::string& uname, User& out)
{
    std::ifstream f(USERS_FILE);
    std::string line;
    while (std::getline(f, line)) {
        User u = User::deserializeBasic(line);
        if (u.username == uname) { out = u; loadContacts(out); return true; }
    }
    return false;
}

void ChatCore::saveUser(const User& u)
{
    std::ifstream fin(USERS_FILE);
    std::ostringstream buf; std::string line; bool upd = false;
    while (std::getline(fin, line)) {
        User r = User::deserializeBasic(line);
        buf << (r.username == u.username ? u.serializeBasic() : line) << '\n';
        if (r.username == u.username) upd = true;
    }
    if (!upd) buf << u.serializeBasic() << '\n';
    std::ofstream(USERS_FILE, std::ios::trunc) << buf.str();
}

/* ---------- “visto” persistence ---------- */
void ChatCore::loadSeen()
{
    seen.clear();
    std::ifstream f(seenFile());
    std::string line;
    while (std::getline(f, line)) {
        auto p = line.find('|'); if (p == std::string::npos) continue;
        seen[line.substr(0, p)] = std::stoll(line.substr(p + 1));
    }
}
void ChatCore::saveSeen() const
{
    std::ofstream f(seenFile(), std::ios::trunc);
    for (auto& [peer, ts] : seen) f << peer << '|' << ts << '\n';
}

/* ---------- login / logout ---------- */
bool ChatCore::loginUser(const std::string& u, const std::string& p)
{
    if (!loadUser(u, current) || current.password != p) return false;
    logged = true; loadSeen(); return true;
}

/* ---------------- perfil ---------------- */
bool ChatCore::updateAvatar(const std::string& path)
{
    if (!logged) return false;
    current.avatar = path;
    saveUser(current);
    return true;
}

bool ChatCore::updatePassword(const std::string& oldP, const std::string& newP)
{
    if (!logged || current.password != oldP || newP.size() < 6) return false;
    current.password = newP; saveUser(current); return true;
}

bool ChatCore::deleteCurrentUser()
{
    if (!logged) return false;

    std::ifstream fin(USERS_FILE);
    std::ostringstream buf; std::string line;
    while (std::getline(fin, line)) {
        if (User::deserializeBasic(line).username != current.username)
            buf << line << '\n';
    }
    std::ofstream(USERS_FILE, std::ios::trunc) << buf.str();

    std::remove(contactsFile(current.username).c_str());
    std::remove(seenFile().c_str());

    logged = false;
    return true;
}

/* ---------------- contactos ---------------- */
std::string ChatCore::contactsFile(const std::string& u) { return u + "_contacts.txt"; }

void ChatCore::loadContacts(User& u)
{
    std::ifstream f(contactsFile(u.username));
    std::string l; while (std::getline(f, l)) if (!l.empty()) u.contacts.push_back(l);
    u.contacts.bubbleSort();
}
void ChatCore::saveContact(const std::string& own, const std::string& c)
{
    std::ofstream(contactsFile(own), std::ios::app) << c << '\n';
}

/* ---------------- historial ---------------- */
std::string ChatCore::historyFile(const std::string& a, const std::string& b) const
{ return (a < b ? a + '_' + b : b + '_' + a) + "_chat.txt"; }

void ChatCore::printHistory(const std::string& a, const std::string& b)
{
    std::ifstream f(historyFile(a, b)); std::string ln;
    while (std::getline(f, ln)) {
        auto m = Message<std::string>::deserialize(ln);
        std::cout << '[' << timestampToStr(m.ts) << "] "
                  << m.from << ": " << m.content << '\n';
    }
}
void ChatCore::removeLastFromHistory(const std::string& a, const std::string& b)
{
    std::string file = historyFile(a, b);
    std::ifstream fin(file); std::ostringstream buf;
    std::string line, last;
    while (std::getline(fin, line)) { buf << line << '\n'; last = line; }
    std::string all = buf.str();
    size_t pos = all.rfind(last + '\n'); if (pos != std::string::npos) all.erase(pos);
    std::ofstream(file, std::ios::trunc) << all;
}

/* ---------- no‑leídos ---------- */
Queue<Message<std::string>>
ChatCore::takeUnread(const std::string& from)
{
    auto it = pending.find(from);
    if (it == pending.end()) return {};

    auto q = std::move(it->second);
    pending.erase(it);
    return q;
}

int ChatCore::unreadCount(const std::string& peer) const
{
    int mem = 0;
    if (auto it = pending.find(peer); it != pending.end()) {
        Queue<Message<std::string>> tmp = it->second;
        while (!tmp.empty()) { tmp.dequeue(); ++mem; }
    }
    time_t last = 0; if (auto it = seen.find(peer); it != seen.end()) last = it->second;

    int fileCnt = 0; std::ifstream fin(historyFile(peer, current.username));
    std::string line;
    while (std::getline(fin, line)) {
        auto msg = Message<std::string>::deserialize(line);
        if (msg.to == current.username && msg.ts > last) ++fileCnt;
    }
    return mem + fileCnt;
}

void ChatCore::markSeen(const std::string& peer)
{
    seen[peer] = time(nullptr);
    saveSeen();
}

bool ChatCore::resetPassword(const std::string& user,
                             const std::string& answer,
                             const std::string& newPass)
{
    if (newPass.size() < 6) return false;

    User u;
    if (!loadUser(user, u)) return false;
    if (u.secA != answer)   return false;

    u.password = newPass;
    saveUser(u);
    return true;
}

/* ---------------- consola opcional ---------------- */
void ChatCore::printer(const std::string& s){ std::cout << " • " << s << '\n'; }
void ChatCore::addContact(){ /* igual que antes */ }
void ChatCore::listContacts(){ /* igual */ }
void ChatCore::chatMenu()  { /* igual */ }
void ChatCore::userMenu()  { /* igual */ }
void ChatCore::login()     { /* igual (modo consola) */ }
void ChatCore::registerUser(){ /* igual */ }
void ChatCore::run()       { /* igual */ }
