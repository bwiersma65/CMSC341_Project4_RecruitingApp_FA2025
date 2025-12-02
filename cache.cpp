// CMSC 341 - Fall 25 - Project 4
#include "cache.h"
Cache::Cache(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    // assign given size into capacity member variable
    m_currentCap = size;

    // if given size is non-prime
    if (!isPrime(m_currentCap)) {
        // finds next prime after given non-prime size
        // returns value between MINPRIME and MAXPRIME
        m_currentCap = findNextPrime(m_currentCap);
    }
    // if given size is prime
    else {
        if (m_currentCap < MINPRIME) m_currentCap = MINPRIME;
        else if (m_currentCap > MAXPRIME) m_currentCap = MAXPRIME;
    }

    // allocates memory for new hash table with initial size of a prime number
    m_currentTable = new Person*[m_currentCap];
    // initialize each element of table with default Person object
    for (int i=0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

    // assigns initial hash function for table
    m_hash = hash;
    // assigns initial collision handling policy for table
    m_currProbing = probing;
    m_newPolicy = probing;

    m_currentSize = 0;
    m_currNumDeleted = 0;

    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;

    m_transferIndex = 0;
}

Cache::~Cache(){
    // check if current table is allocated
    if (m_currentTable != nullptr) {
        // deallocate memory of each table element
        for (int i=0; i < m_currentCap; i++) {
            delete m_currentTable[i];
        }
        // deallocate memory of table itself
        delete [] m_currentTable;
    }
    // check if old table member is allocated
    if (m_oldTable != nullptr) {
        // dealloc memory of each element
        for (int i=0; i < m_oldCap; i++) {
            delete m_oldTable[i];
        }
        // dealloc memory of table itself
        delete [] m_oldTable;
    }
}
// Sets new collision handling policy for table into m_newPolicy member
// This policy is not implemented until the next rehash/transfer operation commences
void Cache::changeProbPolicy(prob_t policy){
    // store newly-requested collision handling policy
    m_newPolicy = policy;
}

bool Cache::insert(Person person){
    // check that Person ID is within valid range
    if ((person.getID() < MINID) || (MAXID < person.getID())) {
        return false;
    }
    // check if Person already exists in table
    else if (getPerson(person.getKey(), person.getID()).getID() == 0) {
        return false;
    }
    else {

    }
    // Run Person's key through hash function and modulo by table capacity to determine
    // initial insertion index
    int insertIndex = m_hash(person.getKey()) % m_currentCap;
}

bool Cache::remove(Person person){
    
}

const Person Cache::getPerson(string key, int ID) const{
    
}
// Looks for person in table; if found, updates its ID and returns true
// If not found, returns false
bool Cache::updateID(Person person, int ID){
    // temp holder for result of getPerson()
    // either holds the person from table, or holds an empty person object
    Person temp = getPerson(person.getKey(), person.getID());
    // empty person object; not found in table
    if (temp.getID() == 0) {
        return false;
    }
    else {
        // Update person'd ID to parameter value
        temp.setID(ID);
        // Hash person object to find unaltered object's location in table
        int index = m_hash(person.getKey()) % m_currentCap;
        // Overwrite unaltered person with updated person
        m_currentTable[index]->operator=(temp);
    }
}
// Returns load factor of current hash table
// Load factor: ratio of occupied buckets (live + deleted nodes) to table capacity
float Cache::lambda() const {
    float lf = (static_cast<float>(m_currentSize))/m_currentCap;
    return lf;
}
// Returns ratio of deleted buckets to occupied (live + deleted) buckets
float Cache::deletedRatio() const {
    float dr = (static_cast<float>(m_currNumDeleted))/m_currentSize;
    return dr;
}

void Cache::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool Cache::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int Cache::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}
