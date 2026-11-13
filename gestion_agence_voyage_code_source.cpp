 // ============================================================
//  Gestion des vols et reservations - Agence de voyage (Cotonou)
//  Application console C++ (compatible Dev-C++ / C++11)
//  Pour Dev-C++ : Outils > Options du compilateur > ajouter
//  "-std=c++11" si necessaire.
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cctype>
using namespace std;

// ---------- Utilitaires ----------
string versString(int n)
{
    ostringstream o;
    o << n;
    return o.str();
}

string enMinuscules(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

string lireLigne(const string& invite)
{
    string s;
    cout << invite;
    getline(cin, s);
    return s;
}

string lireNonVide(const string& invite)
{
    string s;
    do {
        s = lireLigne(invite);
        if (s.empty())
            cout << "  -> Ce champ ne peut pas etre vide.\n";
    } while (s.empty());
    return s;
}

int lireEntier(const string& invite)
{
    while (true) {
        string s = lireLigne(invite);
        istringstream iss(s);
        int n;
        char reste;
        if ((iss >> n) && !(iss >> reste))
            return n;
        cout << "  -> Veuillez saisir un nombre entier valide.\n";
    }
}

double lireReel(const string& invite)
{
    while (true) {
        string s = lireLigne(invite);
        istringstream iss(s);
        double d;
        char reste;
        if ((iss >> d) && !(iss >> reste))
            return d;
        cout << "  -> Veuillez saisir un nombre valide.\n";
    }
}

// ============================================================
//  Classe Passager
// ============================================================
class Passager
{
private:
    int id;
    string nom;
    string prenom;
    string telephone;
    string email;

public:
    Passager(int id, const string& nom, const string& prenom,
             const string& telephone, const string& email)
    {
        if (nom.empty() || prenom.empty())
            throw invalid_argument("Le nom et le prenom sont obligatoires.");
        if (telephone.empty())
            throw invalid_argument("Le telephone est obligatoire.");
        this->id = id;
        this->nom = nom;
        this->prenom = prenom;
        this->telephone = telephone;
        this->email = email;
    }

    int getId() const { return id; }
    string getNom() const { return nom; }
    string getPrenom() const { return prenom; }
    string getTelephone() const { return telephone; }
    string getEmail() const { return email; }
    string nomComplet() const { return prenom + " " + nom; }

    void afficher() const
    {
        cout << "Passager #" << id << " : " << nomComplet()
             << " | Tel : " << telephone
             << " | Email : " << (email.empty() ? "-" : email) << endl;
    }
};

// ============================================================
//  Classe Vol
// ============================================================
class Vol
{
private:
    string numero;
    string depart;
    string destination;
    string date;   // ex : 25/12/2026
    string heure;  // ex : 14:30
    int capacite;
    int placesOccupees;
    double prix;

public:
    Vol(const string& numero, const string& depart, const string& destination,
        const string& date, const string& heure, int capacite, double prix)
    {
        if (numero.empty() || depart.empty() || destination.empty())
            throw invalid_argument("Numero, depart et destination sont obligatoires.");
        if (capacite <= 0)
            throw invalid_argument("La capacite doit etre superieure a 0.");
        if (prix < 0)
            throw invalid_argument("Le prix ne peut pas etre negatif.");
        this->numero = numero;
        this->depart = depart;
        this->destination = destination;
        this->date = date;
        this->heure = heure;
        this->capacite = capacite;
        this->placesOccupees = 0;
        this->prix = prix;
    }

    string getNumero() const { return numero; }
    string getDepart() const { return depart; }
    string getDestination() const { return destination; }
    string getDate() const { return date; }
    string getHeure() const { return heure; }
    int getCapacite() const { return capacite; }
    int getPlacesOccupees() const { return placesOccupees; }
    double getPrix() const { return prix; }

    int placesDisponibles() const { return capacite - placesOccupees; }
    bool estComplet() const { return placesOccupees >= capacite; }

    void occuperPlace()
    {
        if (estComplet())
            throw runtime_error("Le vol " + numero + " est complet.");
        placesOccupees++;
    }

    void libererPlace()
    {
        if (placesOccupees > 0)
            placesOccupees--;
    }

    void afficher() const
    {
        cout << "Vol " << numero << " : " << depart << " -> " << destination
             << " | " << date << " " << heure
             << " | Prix : " << fixed << setprecision(0) << prix << " FCFA"
             << " | Places libres : " << placesDisponibles() << "/" << capacite
             << endl;
    }
};

// ============================================================
//  Enumerations
// ============================================================
enum StatutReservation { EN_ATTENTE, VALIDEE, ANNULEE };
enum StatutRemboursement { AUCUN, REMBOURSE_PARTIEL };

string texteStatut(StatutReservation s)
{
    if (s == EN_ATTENTE) return "EN ATTENTE";
    if (s == VALIDEE) return "VALIDEE";
    return "ANNULEE";
}

string texteRemboursement(StatutRemboursement r)
{
    if (r == REMBOURSE_PARTIEL) return "REMBOURSE (80%)";
    return "AUCUN";
}

// ============================================================
//  Classe Paiement
// ============================================================
class Paiement
{
private:
    int id;
    double montant;
    string mode;
    StatutRemboursement statutRemboursement;
    double montantRembourse;

public:
    Paiement(int id, double montant, const string& mode)
    {
        if (montant <= 0)
            throw invalid_argument("Le montant du paiement doit etre positif.");
        this->id = id;
        this->montant = montant;
        this->mode = mode;
        this->statutRemboursement = AUCUN;
        this->montantRembourse = 0;
    }

    int getId() const { return id; }
    double getMontant() const { return montant; }
    string getMode() const { return mode; }
    StatutRemboursement getStatutRemboursement() const { return statutRemboursement; }
    double getMontantRembourse() const { return montantRembourse; }

    // Regle de remboursement : 80% du montant (20% de frais d'annulation)
    void rembourser()
    {
        montantRembourse = montant * 0.80;
        statutRemboursement = REMBOURSE_PARTIEL;
    }
};

// ============================================================
//  Classe Reservation
// ============================================================
class Reservation
{
private:
    int id;
    Passager* passager;   // association : 1 seul passager
    Vol* vol;             // association : 1 seul vol
    Paiement* paiement;   // association : 0..1 paiement
    StatutReservation statut;

public:
    Reservation(int id, Passager* passager, Vol* vol)
    {
        if (passager == NULL || vol == NULL)
            throw invalid_argument("Une reservation necessite un passager et un vol.");
        this->id = id;
        this->passager = passager;
        this->vol = vol;
        this->paiement = NULL;
        this->statut = EN_ATTENTE;
    }

    ~Reservation() { delete paiement; }

    int getId() const { return id; }
    Passager* getPassager() const { return passager; }
    Vol* getVol() const { return vol; }
    Paiement* getPaiement() const { return paiement; }
    StatutReservation getStatut() const { return statut; }
    bool estAnnulee() const { return statut == ANNULEE; }
    bool estPayee() const { return paiement != NULL; }

    // Paiement => validation de la reservation (regle 6)
    void payer(Paiement* p)
    {
        if (statut == ANNULEE)
            throw runtime_error("Reservation annulee : paiement impossible.");
        if (paiement != NULL)
            throw runtime_error("Cette reservation est deja payee.");
        paiement = p;
        statut = VALIDEE;
    }

    // Annulation (regle 7)
    void annuler()
    {
        if (statut == ANNULEE)
            throw runtime_error("Cette reservation est deja annulee.");
        if (paiement != NULL)
            paiement->rembourser();
        statut = ANNULEE;
        vol->libererPlace();
    }

    void afficherResume() const
    {
        cout << "Reservation #" << id << " | " << passager->nomComplet()
             << " | Vol " << vol->getNumero()
             << " (" << vol->getDepart() << "->" << vol->getDestination() << ")"
             << " | " << texteStatut(statut) << endl;
    }

    void afficherDetails() const
    {
        cout << "----------------------------------------\n";
        cout << "Reservation #" << id << endl;
        cout << "Statut        : " << texteStatut(statut) << endl;
        cout << "Passager      : " << passager->nomComplet()
             << " (id " << passager->getId() << ", tel " << passager->getTelephone() << ")" << endl;
        cout << "Vol           : " << vol->getNumero() << " "
             << vol->getDepart() << " -> " << vol->getDestination() << endl;
        cout << "Date / heure  : " << vol->getDate() << " " << vol->getHeure() << endl;
        cout << "Prix du billet: " << fixed << setprecision(0) << vol->getPrix() << " FCFA" << endl;
        if (paiement != NULL) {
            cout << "Paiement      : " << paiement->getMontant() << " FCFA ("
                 << paiement->getMode() << ")" << endl;
            cout << "Remboursement : " << texteRemboursement(paiement->getStatutRemboursement());
            if (paiement->getStatutRemboursement() == REMBOURSE_PARTIEL)
                cout << " -> " << paiement->getMontantRembourse() << " FCFA";
            cout << endl;
        } else {
            cout << "Paiement      : aucun" << endl;
        }
        cout << "----------------------------------------\n";
    }
};

// ============================================================
//  Classe Agence (gestionnaire central, stocke les collections)
// ============================================================
class Agence
{
private:
    string nom;
    vector<Passager*> passagers;
    vector<Vol*> vols;
    vector<Reservation*> reservations;
    int prochainIdPassager;
    int prochainIdReservation;
    int prochainIdPaiement;

public:
    Agence(const string& nom)
    {
        this->nom = nom;
        prochainIdPassager = 1;
        prochainIdReservation = 1;
        prochainIdPaiement = 1;
    }

    ~Agence()
    {
        for (size_t i = 0; i < reservations.size(); i++) delete reservations[i];
        for (size_t i = 0; i < vols.size(); i++) delete vols[i];
        for (size_t i = 0; i < passagers.size(); i++) delete passagers[i];
    }

    string getNom() const { return nom; }

    // ----- Recherches internes -----
    Passager* trouverPassager(int id) const
    {
        for (size_t i = 0; i < passagers.size(); i++)
            if (passagers[i]->getId() == id) return passagers[i];
        return NULL;
    }

    Vol* trouverVol(const string& numero) const
    {
        string n = enMinuscules(numero);
        for (size_t i = 0; i < vols.size(); i++)
            if (enMinuscules(vols[i]->getNumero()) == n) return vols[i];
        return NULL;
    }

    Reservation* trouverReservation(int id) const
    {
        for (size_t i = 0; i < reservations.size(); i++)
            if (reservations[i]->getId() == id) return reservations[i];
        return NULL;
    }

    // ----- Passagers -----
    int enregistrerPassager(const string& nomP, const string& prenom,
                            const string& tel, const string& email)
    {
        Passager* p = new Passager(prochainIdPassager, nomP, prenom, tel, email);
        passagers.push_back(p);
        prochainIdPassager++;
        return p->getId();
    }

    // ----- Vols -----
    void ajouterVol(const string& numero, const string& depart, const string& dest,
                    const string& date, const string& heure, int capacite, double prix)
    {
        if (trouverVol(numero) != NULL)
            throw runtime_error("Un vol avec ce numero existe deja.");
        vols.push_back(new Vol(numero, depart, dest, date, heure, capacite, prix));
    }

    void afficherVolsDisponibles() const
    {
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            if (!vols[i]->estComplet()) {
                vols[i]->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "Aucun vol disponible.\n";
    }

    void rechercherVol(const string& critere) const
    {
        string c = enMinuscules(critere);
        bool trouve = false;
        for (size_t i = 0; i < vols.size(); i++) {
            Vol* v = vols[i];
            if (enMinuscules(v->getNumero()).find(c) != string::npos ||
                enMinuscules(v->getDepart()).find(c) != string::npos ||
                enMinuscules(v->getDestination()).find(c) != string::npos ||
                enMinuscules(v->getDate()).find(c) != string::npos) {
                v->afficher();
                trouve = true;
            }
        }
        if (!trouve) cout << "Aucun vol ne correspond a la recherche.\n";
    }

    // ----- Reservations -----
    int reserver(int idPassager, const string& numeroVol)
    {
        // Regle 1 : passager enregistre
        Passager* p = trouverPassager(idPassager);
        if (p == NULL)
            throw runtime_error("Passager non enregistre : reservation impossible.");

        Vol* v = trouverVol(numeroVol);
        if (v == NULL)
            throw runtime_error("Vol introuvable.");

        // Regle 3 : pas de double reservation sur le meme vol
        for (size_t i = 0; i < reservations.size(); i++) {
            Reservation* r = reservations[i];
            if (!r->estAnnulee() && r->getPassager() == p && r->getVol() == v)
                throw runtime_error("Ce passager a deja reserve ce vol.");
        }

        // Regle 2 : capacite (leve une exception si complet)
        v->occuperPlace();

        Reservation* r = new Reservation(prochainIdReservation, p, v);
        reservations.push_back(r);
        prochainIdReservation++;
        return r->getId();
    }

    // ----- Paiement -----
    void enregistrerPaiement(int idReservation, const string& mode)
    {
        Reservation* r = trouverReservation(idReservation);
        if (r == NULL)
            throw runtime_error("Reservation introuvable.");
        // Regle 5 : verifiee dans Reservation::payer
        Paiement* p = new Paiement(prochainIdPaiement, r->getVol()->getPrix(), mode);
        try {
            r->payer(p);
        } catch (...) {
            delete p;
            throw;
        }
        prochainIdPaiement++;
    }

    // ----- Annulation -----
    void annulerReservation(int idReservation)
    {
        Reservation* r = trouverReservation(idReservation);
        if (r == NULL)
            throw runtime_error("Reservation introuvable.");
        r->annuler();
    }

    // ----- Consultation -----
    void consulterReservations() const
    {
        if (reservations.empty()) {
            cout << "Aucune reservation.\n";
            return;
        }
        for (size_t i = 0; i < reservations.size(); i++)
            reservations[i]->afficherResume();
    }

    void afficherInfosReservation(int id) const
    {
        Reservation* r = trouverReservation(id);
        if (r == NULL)
            throw runtime_error("Reservation introuvable.");
        r->afficherDetails();
    }

    void afficherPassagers() const
    {
        if (passagers.empty()) {
            cout << "Aucun passager enregistre.\n";
            return;
        }
        for (size_t i = 0; i < passagers.size(); i++)
            passagers[i]->afficher();
    }
};

// ============================================================
//  Menu et programme principal
// ============================================================
void afficherMenu()
{
    cout << "\n========================================\n";
    cout << "   AGENCE DE VOYAGE - COTONOU\n";
    cout << "========================================\n";
    cout << " 1. Enregistrer un passager\n";
    cout << " 2. Ajouter un vol\n";
    cout << " 3. Afficher les vols disponibles\n";
    cout << " 4. Rechercher un vol\n";
    cout << " 5. Effectuer une reservation\n";
    cout << " 6. Enregistrer un paiement\n";
    cout << " 7. Annuler une reservation\n";
    cout << " 8. Consulter les reservations\n";
    cout << " 9. Afficher les informations d'une reservation\n";
    cout << " 0. Quitter\n";
    cout << "----------------------------------------\n";
}

int main()
{
    Agence agence("Agence Cotonou");
    int choix = -1;

    while (choix != 0) {
        afficherMenu();
        choix = lireEntier("Votre choix : ");

        try {
            switch (choix) {
            case 1: {
                string nom = lireNonVide("Nom : ");
                string prenom = lireNonVide("Prenom : ");
                string tel = lireNonVide("Telephone : ");
                string email = lireLigne("Email (facultatif) : ");
                int id = agence.enregistrerPassager(nom, prenom, tel, email);
                cout << "Passager enregistre avec l'identifiant " << id << ".\n";
                break;
            }
            case 2: {
                string numero = lireNonVide("Numero du vol : ");
                string depart = lireNonVide("Ville de depart : ");
                string dest = lireNonVide("Destination : ");
                string date = lireNonVide("Date (jj/mm/aaaa) : ");
                string heure = lireNonVide("Heure (hh:mm) : ");
                int capacite = lireEntier("Capacite (places) : ");
                double prix = lireReel("Prix du billet (FCFA) : ");
                agence.ajouterVol(numero, depart, dest, date, heure, capacite, prix);
                cout << "Vol ajoute avec succes.\n";
                break;
            }
            case 3:
                agence.afficherVolsDisponibles();
                break;
            case 4: {
                string critere = lireNonVide("Rechercher (numero, depart, destination ou date) : ");
                agence.rechercherVol(critere);
                break;
            }
            case 5: {
                int idP = lireEntier("Identifiant du passager : ");
                string numero = lireNonVide("Numero du vol : ");
                int idR = agence.reserver(idP, numero);
                cout << "Reservation #" << idR << " creee (EN ATTENTE de paiement).\n";
                break;
            }
            case 6: {
                int idR = lireEntier("Numero de la reservation : ");
                string mode = lireNonVide("Mode de paiement (Especes / Mobile Money / Carte) : ");
                agence.enregistrerPaiement(idR, mode);
                cout << "Paiement enregistre. Reservation VALIDEE.\n";
                break;
            }
            case 7: {
                int idR = lireEntier("Numero de la reservation a annuler : ");
                agence.annulerReservation(idR);
                cout << "Reservation annulee.\n";
                agence.afficherInfosReservation(idR);
                break;
            }
            case 8:
                agence.consulterReservations();
                break;
            case 9: {
                int idR = lireEntier("Numero de la reservation : ");
                agence.afficherInfosReservation(idR);
                break;
            }
            case 0:
                cout << "Au revoir !\n";
                break;
            default:
                cout << "Choix invalide, veuillez recommencer.\n";
            }
        } catch (const exception& e) {
            cout << "Erreur : " << e.what() << endl;
        }
    }
    return 0;
}
