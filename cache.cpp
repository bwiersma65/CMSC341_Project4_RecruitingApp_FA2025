// CMSC 341 - Fall 25 - Project 4
#include "cache.h"
Cache::Cache(int size, hash_fn hash, prob_t probing = DEFPOLCY){
    // assign given size into capacity member variable
    m_currentCap = size;

    // if given size is non-prime
    if (!isPrime(m_currentCap)) {
        // if less than minimum
        if (m_currentCap < MINPRIME) {
            // sets capacity to MINPRIME
            m_currentCap = findNextPrime(m_currentCap);
        }
        // else if greater than maximum
        else if (m_currentCap > MAXPRIME) {
            m_currentCap = MAXPRIME;
        }
        // non-prime but falls within valid range
        else {
            // finds next prime after given non-prime size
            // returns value between MINPRIME and MAXPRIME inclusive
            m_currentCap = findNextPrime(m_currentCap);
        }
    }
    // if given size is prime
    else {
        // if less than minimum
        if (m_currentCap < MINPRIME) {
            // sets capacity to MINPRIME
            m_currentCap = findNextPrime(m_currentCap);
        }
        // else if greater than maximum
        else if (m_currentCap > MAXPRIME) {
            m_currentCap = MAXPRIME;
        }
    }

    // allocates memory for new hash table with initial size of a prime number
    m_currentTable = new Person*[m_currentCap];
    // initialize each element of table with nullptr
    for (int i=0; i < m_currentCap; i++) {
        m_currentTable[i] = nullptr;
    }

    // assigns initial hash function for table
    m_hash = hash;
    // assigns initial collision handling policy for table
    m_currProbing = probing;
    m_newPolicy = m_currProbing;

    m_currentSize = 0;
    m_currNumDeleted = 0;

    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = DEFPOLCY;

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

// If deleted person matching parameter exists in table, parameter will still be inserted
bool Cache::insert(Person person){
    bool success = false;
    // If this insertion would initiate a rehash to increase table capacity but capacity is already MAXPRIME,
    // do not commence with insert
    if ((m_currentCap==MAXPRIME) && ((static_cast<float>(m_currentSize+1)/m_currentCap)>0.5)) {
        return false;
    }

    // local holders for identifying data //
    string key = person.getKey();
    int ID = person.getID();
    ////////////////////////////////////////

    /*
    Validate parameters
    */
    // check that Person ID is within valid range
    if ((ID < MINID) || (MAXID < ID)) {
        return false;
    }
    // check if Person already exists in table
    else if (getPerson(key, ID).getID() != 0) {
        return false;
    }
    // table is full and cannot be resized
    else if (m_currentSize == MAXPRIME) {
        return false;
    }

    // index to insert new person
    int insertIndex;
    /*
    Probing sequence
    - on first iteration (i=0) probing helper returns first hash index (m_hash(key) % m_currentCap)
    - if that bucket index is empty-since-start (null) or empty-since-delete (m_used=false), insert commences
    - if not, attempt iterates (i++) and try again with probing scheme provided by probingHelper
    - if for-loop ends and success=false, table from first hash index to m_currentCap was searched and no
      suitable bucket was found to insert into
    */
    for (int i=0; i < m_currentCap; i++) {
        insertIndex = probingHelper(m_currentTable, key, i);
        // bucket is empty-since-start; good for insert
        if (m_currentTable[insertIndex] == nullptr) {
            // Create new person with parameter and assign to hashed bucket
            m_currentTable[insertIndex] = new Person(key, ID, true);
            
            success = true;
            m_currentSize++;
            break;
        }
        // bucket is empty-since-delete
        else if (m_currentTable[insertIndex]->getUsed() == false) {
            // copy data from parameter into existing deleted node
            *(m_currentTable[insertIndex]) = person;
            // Make sure m_used is set to true
            m_currentTable[insertIndex]->setUsed(true);
            // insertion successful
            success = true;
            m_currentSize++;
            m_currNumDeleted--;
            break;
        }
    }
    
    // check if rehash has already been initiated previously
    if (m_transferIndex != 0) {
        rehash();
    }
    // no rehashes are currently in progress
    // if insert was success, check if rehash initiation is needed
    else if (success) {
        // load factor is ratio of total number of occupied buckets to table size
        float loadFactor = lambda();
        // If load factor exceeds 50%, initiate rehash
        if (loadFactor >= 0.5) {
            rehash();
        }
    }

    // Returns true if parameter was inserted to hash table successfully
    // Returns false if not
    return success;
}

bool Cache::remove(Person person){
    bool success = false;

    // local holders for identifying data for parameter //
    string key = person.getKey();
    int ID = person.getID();
    //////////////////////////////////////////////////////

    /*
    Validate parameters
    */
    // check if Person does not exist in table (current, or old if it exists)
    if (getPerson(key, ID).getID() == 0) {
        return false;
    }

    /*
    Search current table
    */
    int index;
    for (int i=0; i < m_currentCap; i++) {
        index = probingHelper(m_currentTable, key, i);
        // Check if initial bucket element is non-null and is live data
        if ((m_currentTable[index] != nullptr) && (m_currentTable[index]->getUsed() == true)) {
            // Check if intitial element matches parameter data
            if ((m_currentTable[index]->getID() == ID) && (m_currentTable[index]->getKey() == key)) {
                // delete element by setting m_used to false
                m_currentTable[index]->setUsed(false);
                // removal was successful
                success = true;
                m_currNumDeleted++;
                break;
            }
        }
    }
    /*
    Search old table
    */
    // if search failed and the old table exists
    if ((m_oldTable != nullptr) && !success) {
        for (int i=0; i < m_oldCap; i++) {
            index = probingHelper(m_oldTable, key, i);
            // Check if initial bucket element is non-null and is live data
            if ((m_oldTable[index] != nullptr) && (m_oldTable[index]->getUsed() == true)) {
                // Check if intitial element matches parameter data
                if ((m_oldTable[index]->getID() == ID) && (m_oldTable[index]->getKey() == key)) {
                    // delete element by setting m_used to false
                    m_oldTable[index]->setUsed(false);
                    // removal was successful
                    success = true;
                    m_oldNumDeleted++;
                    break;
                }
            }
        }
    }

    // check if rehash has already been initiated previously
    if (m_transferIndex != 0) {
        rehash();
    }
    // no rehashes are currently in progress
    // if removal was success, check if rehash initiation is needed
    else if (success) {
        // The deleted ratio is the number of deleted buckets divided by the number of occupied buckets
        float deleteRatio = deletedRatio();
        // If delete ratio exceeds 80%, initiate rehash
        if (deleteRatio >= 0.8) {
            rehash();
        }
    }
    
    return success;
}
// Returns copy of Person with given <key,ID> pair if Person found live at mapped bucket index
// i.e. does not return copy if bucket is empty-since-start (nullptr) or empty-since-delete (m_used==false)
const Person Cache::getPerson(string key, int ID) const{
    int index;
    //bool found = false;
    Person temp = Person();
    // search indices mapped by probing method for Person with matching key + ID pair
    // if bucket is occupied by live data, handle the collision and keep searching
    // if deleted bucket matching data, or empty-since-start bucket is encountered, 
    // or entire table is searched without finding match, person with parameter <key,ID> pair does not exist

    /*
    Search current table
    */
    for (int i = 0; i < m_currentCap; i++) {
        // determine bucket index using current probing method
        index = probingHelper(m_currentTable, key, i);

        // mapped bucket is empty-since-start
        // Person does not exist in this table
        // Look next in old table if it exists
        if (m_currentTable[index] == nullptr) {
            //return Person();
            //found = false;
            break;
        }

        // // mapped bucket is empty-since-delete
        // else if (!(m_currentTable[index]->getUsed())) {
        //     // sought Person is in table but has been deleted
        //     if ((m_currentTable[index]->getID() == ID) && (m_currentTable[index]->getKey().compare(key) == 0)) {
        //         //found = false;
        //         break;
        //     }
        // }

        // mapped bucket contains live Person matching parameters
        else if ((m_currentTable[index]->getUsed()) && (m_currentTable[index]->getID() == ID) && (m_currentTable[index]->getKey().compare(key) == 0)) {
            // return copy of found Person
            return *m_currentTable[index];
        }
    }
    // if old table exists
    if (m_oldTable != nullptr) {
        /*
        Search old table
        */
        for (int i = 0; i < m_oldCap; i++) {
            // determine bucket index using current probing method
            index = probingHelper(m_oldTable, key, i);

            // mapped bucket is empty-since-start
            // Person does not exist in table
            // returns empty person object
            if (m_oldTable[index] == nullptr) {
                //return Person();
                break;
            }

            // // mapped bucket is empty-since-delete
            // else if (!(m_oldTable[index]->getUsed())) {
            //     // sought Person is in table but has been deleted
            //     if ((m_oldTable[index]->getID() == ID) && (m_oldTable[index]->getKey().compare(key) == 0)) {
            //         //return Person();
            //         break;
            //     }
            // }

            // mapped bucket contains live Person matching parameters
            else if ((m_oldTable[index]->getID() == ID) && (m_oldTable[index]->getKey().compare(key) == 0)) {
                // return copy of found Person
                return *m_oldTable[index];
            }
        }
    }    

    // current table and/or old table searched and no person found;
    // return empty Person, to indicate nothing was found
    return Person();
}
// Looks for person alive in table (current and old); if found, updates its ID and returns true
// If not found, returns false
bool Cache::updateID(Person person, int ID){
    // temp holder for result of getPerson()
    // either holds copy of person from table, or holds an empty person object if not found
    Person personTemp = getPerson(person.getKey(), person.getID());

    // empty person object; person not found in table
    if ((personTemp.getID() == 0)) {
        return false;
    }

    // found as live data in table
    /*
    Search current table
    */
    else {
        int index;
        for (int i=0; i < m_currentCap; i++) {
            // determine bucket index
            index = probingHelper(m_currentTable, person.getKey(), i);
            // check if matching Person object is found yet
            if ((m_currentTable[index] != nullptr) && (!(m_currentTable[index]->getUsed())) && (m_currentTable[index]->getID() == person.getID()) 
                    && (m_currentTable[index]->getKey().compare(person.getKey()) == 0)) {
                // Update ID member of person in table
                m_currentTable[index]->setID(ID);

                return true;
            }
        }

        /*
        Search old table
        */
        for (int i=0; i < m_oldCap; i++) {
            // determine bucket index
            index = probingHelper(m_oldTable, person.getKey(), i);
            // check if matching Person object is found yet
            if ((m_oldTable[index] != nullptr) && (!(m_oldTable[index]->getUsed())) && (m_oldTable[index]->getID() == person.getID()) 
                    && (m_oldTable[index]->getKey().compare(person.getKey()) == 0)) {
                // Update ID member of person in table
                m_oldTable[index]->setID(ID);

                return true;
            }
        }
    }

    return false;
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

/******************************************
 * Private function definitions go here! *
******************************************/
// calls individual probing helper based on current collision-handling policy
// returns bucket index mapped to for Person with given hashcode and attempt iteration "i"
int Cache::probingHelper(Person** table, string key, int i) const {
    // result of hashing key
    int hash = m_hash(key);
    // Checks which table's probing policy to use
    prob_t probPolicy;
    int capacity;
    if (table == m_currentTable) {
        probPolicy = m_currProbing;
        capacity = m_currentCap;
    }
    else if (table == m_oldTable) {
        probPolicy = m_oldProbing;
        capacity = m_oldCap;
    }

    switch (probPolicy) {
        case LINEAR:
            return linearProbe(hash, i, capacity);
            break;
        case QUADRATIC:
            return quadProbe(hash, i, capacity);
            break;
        case DOUBLEHASH:
            return doubleHashProbe(hash, i, capacity);
            break;
    }
    // error case: should not trigger
    return -1;
}
// returns bucket index directly mapped to by hash function
// if index occupied, will increment by 1 for each iteration
int Cache::linearProbe(int hash, int i, int capacity) const {
    if (i==0) {
        return hash % capacity;
    }
    else {
        return (hash + i) % capacity;
    }
}
// returns bucket index
// if index occupied, increments by iteration and square of iteration
int Cache::quadProbe(int hash, int i, int capacity) const {
    if (i==0) {
        return hash % capacity;
    }
    else {
        return (hash + i + (i*i)) % capacity;
    }
}
// returns bucket index
// if index occupied, performs a double-hash to determine index
int Cache::doubleHashProbe(int hash, int i, int capacity) const {
    if (i==0) {
        return hash % capacity;
    }
    else {
        return ((hash % capacity) + (i * (11 - (hash % 11)))) % capacity;
    }
}
// 
void Cache::rehash() {
    // Scan 1/4 of old table at a time
    // When encountering live node (m_used==true), insert to current table

    // Transfer has not started yet
    // Initiate rehash by swapping tables
    if (m_transferIndex==0) {
        // new table capacity must be smallest prime greater than 4 times current number of data points (number of occupied buckets - number deleted buckets)
        int newTableCap = findNextPrime(4*(m_currentSize-m_currNumDeleted));
        // transfer current table to old table
        m_oldTable = m_currentTable;
        m_oldCap = m_currentCap;
        m_oldSize = m_currentSize;
        m_oldNumDeleted = m_currNumDeleted;
        m_oldProbing = m_currProbing;
        // alloc memory for new table using new table capacity
        m_currentTable = new Person*[newTableCap];
        m_currentCap = newTableCap;
        // change policy if policy change was previously requested
        // otherwise retains policy
        m_currProbing = m_newPolicy;

        // initialize each element of table with nullptr
        for (int i=0; i < m_currentCap; i++) {
            m_currentTable[i] = nullptr;
        }
    }
    
    int quarter = m_oldCap/4;

    // Start this rehash where last rehash left off (index 0 if new rehash)
    // Transfer buckets contents until a quarter of table has been transferred
    for (m_transferIndex; m_transferIndex < (m_transferIndex+quarter); m_transferIndex++) {
        // check if data is live
        if ((m_oldTable[m_transferIndex] != nullptr) && (m_oldTable[m_transferIndex]->getUsed())) {
            // insert live data into current table
            insert(*(m_oldTable[m_transferIndex]));
            // delete data from old table
            remove(*(m_oldTable[m_transferIndex]));
        }
    }

    // 25% of data has been transferred, m_transferIndex currently holds value of index to next be transferred

    // after four transfers, whatever is remaining must be transferred to new table
    if (m_transferIndex > (quarter*3)) {
        while (m_transferIndex < m_oldCap) {
            // check if data is live or not
            if ((m_oldTable[m_transferIndex] != nullptr) && (m_oldTable[m_transferIndex]->getUsed())) {
                // insert live data into current table
                insert(*(m_oldTable[m_transferIndex]));
                // delete data from old table
                remove(*(m_oldTable[m_transferIndex]));
            }
            m_transferIndex++;
        }
        // All live data from old table has been transferred:
        // delete and clear old table
        delete [] m_oldTable;
        m_oldCap = 0;
        m_oldSize = 0;
        m_oldNumDeleted = 0;
        m_oldProbing = DEFPOLCY;

        // reset transfer index to indicate rehashing is finished
        m_transferIndex = 0;
    }
}