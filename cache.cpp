// CMSC 341 - Fall 25 - Project 4
#include "cache.h"
Cache::Cache(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    // assign given size into capacity member variable
    int m_currentCap = size;

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
        m_currentTable[i] = new Person();
    }

    // assigns initial hash function for table
    m_hash = hash;
    // assigns initial collision handling policy for table
    m_currProbing = probing;

    m_currentSize = 0;
    m_currNumDeleted = 0;

    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;

    m_transferIndex = 0;
}

Cache::~Cache(){
    for (int i=0; i < m_currentCap; i++) {
        delete m_currentTable[i];
    }

    delete m_currentTable;
}

void Cache::changeProbPolicy(prob_t policy){
    
}

bool Cache::insert(Person person){
    
}

bool Cache::remove(Person person){
    
}

const Person Cache::getPerson(string key, int ID) const{
    
}

bool Cache::updateID(Person person, int ID){
    
}

float Cache::lambda() const {
      
}

float Cache::deletedRatio() const {
    
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
