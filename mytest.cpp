/* 
 * File:    mytest.cpp
 * Author:  Darian Afkhami
 * Course:  CMSC 341
 * Prof:    Professor Kartchner
 *
 * Description:
 *   Comprehensive test suite for Project 4 Cache class.
 */

#include <iostream>
#include "cache.h"
#include <math.h>
#include <algorithm>
#include <random>
#include <vector>
using namespace std;


enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(){}
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = mt19937(10);// 10 is the fixed seed value
            m_unidist = uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = mt19937(10);// 10 is the fixed seed value
            m_uniReal = uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = mt19937(seedNum);
    }
    void init(int min, int max){
        m_min = min;
        m_max = max;
        m_type = UNIFORMINT;
        m_generator = mt19937(10);// 10 is the fixed seed value
        m_unidist = uniform_int_distribution<>(min,max);
    }
    void getShuffle(vector<int> & array){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // this function provides a list of all values between min and max
        // in a random order, this function guarantees the uniqueness
        // of every value in the list
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = floor(result*100.0)/100.0;
        return result;
    }

    string getRandString(int size){
        // the parameter size specifies the length of string we ask for
        // to use ASCII char the number range in constructor must be set to 97 - 122
        // and the Random type must be UNIFORMINT (it is default in constructor)
        string output = "";
        for (int i=0;i<size;i++){
            output = output + (char)getRandNum();
        }
        return output;
    }
    
    int getMin(){return m_min;}
    int getMax(){return m_max;}
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    random_device m_device;
    mt19937 m_generator;
    normal_distribution<> m_normdist;//normal distribution
    uniform_int_distribution<> m_unidist;//integer uniform distribution
    uniform_real_distribution<double> m_uniReal;//real uniform distribution
};

/* Hash function copied from driver.cpp */
unsigned int hashCode(const string str) {
    unsigned int val = 0;
    const unsigned int thirtyThree = 33;
    for (int i = 0; i < (int)str.length(); i++) {
        val = val * thirtyThree + str[i];
    }
    return val;
}

/* Helper: use Random class to get an int in [min, max] */
int getRandInRange(Random &rng, int min, int max) {
    // Only (re)initialize if the range actually changed.
    // This keeps the generator advancing across calls and avoids duplicates in a row.
    if (rng.getMin() != min || rng.getMax() != max) {
        rng.init(min, max);
    }
    return rng.getRandNum();
}


/* Tester class declaration */
class Tester {
public:
    bool testInsertNonColliding();
    bool testInsertWithCollisions();
    bool testFindErrorCase();
    bool testFindNonColliding();
    bool testFindColliding();
    bool testRemoveNonColliding();
    bool testRemoveColliding();
    bool testRehashTriggerLoadFactor();
    bool testRehashCompletionLoadFactor();
    bool testRehashTriggerDeletedRatio();
    bool testRehashCompletionDeletedRatio();
    bool testInsertDeletedReuse();
    bool testUpdateIDSuccess();
    bool testUpdateIDFailure();
    bool testInvalidIDInsertion();
    bool testPolicyChangeAfterRehash();
    bool testQuadraticWrap();
    bool testDoubleHashStepsCorrect();
    bool testFindStopsOnNull();
};

/* Test: Insert non-colliding keys */
bool Tester::testInsertNonColliding() {
    Random rng(0, 1);
    hash_fn h = hashCode;
    int tableSize = 101;
    Cache c(tableSize, h, LINEAR);

    const int NUM = 50;
    string keys[NUM];
    int ids[NUM];
    int i = 0;

    for (i = 0; i < NUM; i++) {
        keys[i] = "K" + to_string(getRandInRange(rng, 100000, 999999));
        ids[i] = getRandInRange(rng, MINID, MAXID);
        int j = 0;
        for (j = 0; j < i; j++) {
            if (keys[i] == keys[j]) {
                i--;
                break;
            }
        }
    }

    for (i = 0; i < NUM; i++) {
        Person p(keys[i], ids[i], true);
        if (!c.insert(p)) {
            return false;
        }
        Person found = c.getPerson(keys[i], ids[i]);
        if (!found.getUsed()) {
            return false;
        }
        if (found.getKey() != keys[i] || found.getID() != ids[i]) {
            return false;
        }
    }

    return true;
}

/* Test: Insert colliding keys */
bool Tester::testInsertWithCollisions() {
    // Checks that insertion works correctly when many keys collide.
    // Doesn't assume that all (key, ID) pairs are unique; if insert() returns true, the Person must be findable by getPerson().
    Random keyRng(1000, 9999);      // for suffix of keys
    Random idRng(MINID, MAXID);     // for IDs
    Cache c(101, hashCode, QUADRATIC);

    const int NUM = 50;
    string keys[NUM];
    int ids[NUM];
    bool inserted[NUM];
    int i = 0;

    // Generate colliding keys (same prefix) and random IDs.
    for (i = 0; i < NUM; i++) {
        keys[i] = "AA" + to_string(keyRng.getRandNum());
        ids[i] = idRng.getRandNum();
        inserted[i] = false;
    }

    int insertedCount = 0;

    // Insert and check that any successfully inserted element is retrievable.
    for (i = 0; i < NUM; i++) {
        cout << "Inside for loop" << endl;//
        Person p(keys[i], ids[i], true);
        bool ok = c.insert(p);
        cout << "After insert" << endl;
        inserted[i] = ok;
        if (ok) {
            insertedCount++;
            Person found = c.getPerson(keys[i], ids[i]);
            cout << "After find" << endl;
            if (!found.getUsed()) {
                return false;
            }
            if (found.getKey() != keys[i] || found.getID() != ids[i]) {
                return false;
            }
        }
        cout << "End of for loop" << endl;//
    }

    // At least half should have been inserted.
    if (insertedCount < NUM / 2) {
        return false;
    }

    return true;
}



/* Test: Find error case */
bool Tester::testFindErrorCase() {
    Cache c(101, hashCode, LINEAR);
    Person result = c.getPerson("MissingKey", 12345);
    return !result.getUsed();
}

/* Test: Find non-colliding */
bool Tester::testFindNonColliding() {
    Random rng(0, 1);
    Cache c(101, hashCode, LINEAR);

    const int NUM = 50;
    string keys[NUM];
    int ids[NUM];
    int i = 0;

    for (i = 0; i < NUM; i++) {
        keys[i] = "NC" + to_string(getRandInRange(rng, 100000, 999999));
        ids[i] = getRandInRange(rng, MINID, MAXID);

        int j = 0;
        for (j = 0; j < i; j++) {
            if (keys[i] == keys[j]) {
                i--;
                break;
            }
        }
    }

    for (i = 0; i < NUM; i++) {
        Person p(keys[i], ids[i], true);
        if (!c.insert(p)) {
            return false;
        }
    }

    for (i = 0; i < NUM; i++) {
        Person found = c.getPerson(keys[i], ids[i]);
        if (!found.getUsed()) {
            return false;
        }
        if (found.getKey() != keys[i] || found.getID() != ids[i]) {
            return false;
        }
    }

    return true;
}

/* Test: Find colliding */
bool Tester::testFindColliding() {
    // Checks that find works correctly with colliding keys.
    Random keyRng(1000, 9999);
    Random idRng(MINID, MAXID);
    Cache c(101, hashCode, QUADRATIC);

    const int NUM = 50;
    string keys[NUM];
    int ids[NUM];
    bool inserted[NUM];
    int i = 0;

    // Generate colliding keys and random IDs.
    for (i = 0; i < NUM; i++) {
        keys[i] = "COLL" + to_string(keyRng.getRandNum());
        ids[i] = idRng.getRandNum();
        inserted[i] = false;
    }

    // Insert them, track which ones actually made it into the table.
    for (i = 0; i < NUM; i++) {
        Person p(keys[i], ids[i], true);
        bool ok = c.insert(p);
        inserted[i] = ok;
    }

    // For every successfully inserted element, getPerson must find it.
    for (i = 0; i < NUM; i++) {
        if (inserted[i]) {
            Person found = c.getPerson(keys[i], ids[i]);
            if (!found.getUsed()) {
                return false;
            }
            if (found.getKey() != keys[i] || found.getID() != ids[i]) {
                return false;
            }
        }
    }

    return true;
}



/* Test: Remove non-colliding */
bool Tester::testRemoveNonColliding() {
    Random rng(0, 1);
    Cache c(101, hashCode, LINEAR);

    const int NUM = 50;
    const int REMOVE_COUNT = 10;
    string keys[NUM];
    int ids[NUM];
    int i = 0;

    for (i = 0; i < NUM; i++) {
        keys[i] = "REM" + to_string(getRandInRange(rng, 100000, 999999));
        ids[i] = getRandInRange(rng, MINID, MAXID);

        int j = 0;
        for (j = 0; j < i; j++) {
            if (keys[i] == keys[j]) {
                i--;
                break;
            }
        }
    }

    for (i = 0; i < NUM; i++) {
        if (!c.insert(Person(keys[i], ids[i], true))) {
            return false;
        }
    }

    for (i = 0; i < REMOVE_COUNT; i++) {
        if (!c.remove(Person(keys[i], ids[i], true))) {
            return false;
        }
        if (c.getPerson(keys[i], ids[i]).getUsed()) {
            return false;
        }
    }

    for (i = REMOVE_COUNT; i < NUM; i++) {
        if (!c.getPerson(keys[i], ids[i]).getUsed()) {
            return false;
        }
    }

    return true;
}

/* Test: Remove colliding */
bool Tester::testRemoveColliding() {
    // Use separate Random generators for keys and IDs to avoid interaction with getRandInRange() and to keep things simple.
    Random keyRng(1000, 9999);
    Random idRng(MINID, MAXID);
    Cache c(101, hashCode, QUADRATIC);

    const int NUM = 50;
    const int TARGET_REMOVE = 20;

    string keys[NUM];
    int ids[NUM];
    bool inserted[NUM];
    bool removed[NUM];

    int i = 0;

    // Generate colliding keys (same prefix) and random IDs.
    for (i = 0; i < NUM; i++) {
        keys[i] = "RC" + to_string(keyRng.getRandNum());
        ids[i] = idRng.getRandNum();
        inserted[i] = false;
        removed[i] = false;
    }

    // Insert all and record which ones actually made it into the table.
    for (i = 0; i < NUM; i++) {
        Person p(keys[i], ids[i], true);
        bool ok = c.insert(p);
        inserted[i] = ok;
    }

    // Remove up to TARGET_REMOVE inserted elements.
    int removedCount = 0;
    for (i = 0; i < NUM && removedCount < TARGET_REMOVE; i++) {
        if (inserted[i]) {
            if (!c.remove(Person(keys[i], ids[i], true))) {
                return false;
            }
            removed[i] = true;
            removedCount++;

            // The removed element must no longer be findable.
            Person after = c.getPerson(keys[i], ids[i]);
            if (after.getUsed()) {
                return false;
            }
        }
    }

    // Expect to be able to remove at least a few elements.
    if (removedCount == 0) {
        return false;
    }

    // All removed elements must be gone; all other inserted elements must remain.
    for (i = 0; i < NUM; i++) {
        Person found = c.getPerson(keys[i], ids[i]);

        if (removed[i]) {
            if (found.getUsed()) {
                return false;
            }
        }
        else if (inserted[i]) {
            if (!found.getUsed()) {
                return false;
            }
        }
        // If !inserted[i] and !removed[i], don't care since it never made it in.
    }

    return true;
}



/* Test: Rehash trigger (load factor) */
bool Tester::testRehashTriggerLoadFactor() {
    Cache c(101, hashCode, LINEAR);
    c.changeProbPolicy(DOUBLEHASH);

    Random rng(0, 1);
    int i = 0;

    while (c.m_oldTable == NULL && i < 400) {
        string key = "LF" + to_string(getRandInRange(rng, 100000, 999999)) + "_" + to_string(i);
        int id = getRandInRange(rng, MINID, MAXID);
        c.insert(Person(key, id, true));
        i++;
    }

    if (c.m_oldTable == NULL) {
        return false;
    }

    if (c.m_currProbing != DOUBLEHASH) {
        return false;
    }

    return true;
}


/* Test: Rehash completion (load factor) */
bool Tester::testRehashCompletionLoadFactor() {
    Cache c(101, hashCode, LINEAR);

    Random rng(0, 1);
    string keys[150];
    int ids[150];
    int i = 0;

    for (i = 0; i < 150; i++) {
        keys[i] = "LFC" + to_string(getRandInRange(rng, 100000, 999999));
        ids[i] = getRandInRange(rng, MINID, MAXID);

        int j = 0;
        for (j = 0; j < i; j++) {
            if (keys[i] == keys[j]) {
                i--;
                break;
            }
        }
    }

    int inserted = 0;
    for (i = 0; i < 150; i++) {
        if (!c.insert(Person(keys[i], ids[i], true))) {
            return false;
        }
        inserted++;
        if (c.m_oldTable != NULL) {
            break;
        }
    }

    if (c.m_oldTable == NULL) {
        return false;
    }

    int k = i + 1;
    for (k = i + 1; k < 150 && c.m_oldTable != NULL; k++) {
        c.insert(Person(keys[k], ids[k], true));
    }

    if (c.m_oldTable != NULL) {
        return false;
    }

    for (i = 0; i < k; i++) {
        if (!c.getPerson(keys[i], ids[i]).getUsed()) {
            return false;
        }
    }

    return true;
}

/* Test: Rehash trigger (deleted ratio) */
bool Tester::testRehashTriggerDeletedRatio() {
    // Use a larger table so load factor stays well below 0.5 and rehashing (if any) must be due to deleted ratio.
    Cache c(401, hashCode, LINEAR);
    Random keyRng(10000, 99999);
    Random idRng(MINID, MAXID);

    const int TARGET_INSERT = 60;
    string keys[TARGET_INSERT];
    int ids[TARGET_INSERT];
    bool inserted[TARGET_INSERT];
    int i = 0;

    for (i = 0; i < TARGET_INSERT; i++) {
        inserted[i] = false;
    }

    int count = 0;
    int attempts = 0;

    // Insert until there is TARGET_INSERT successful inserts or give up after 3 * TARGET_INSERT attempts.
    while (count < TARGET_INSERT && attempts < TARGET_INSERT * 3) {
        string key = "DR" + to_string(keyRng.getRandNum());
        int id = idRng.getRandNum();
        Person p(key, id, true);

        if (c.insert(p)) {
            keys[count] = key;
            ids[count] = id;
            inserted[count] = true;
            count++;
        }

        attempts++;
    }

    // Need a decent number of data points.
    if (count < 40) {
        return false;
    }

    // Load factor should still be below 0.5, so no rehash should have been triggered yet.
    if (c.m_oldTable != NULL) {
        return false;
    }

    // Delete enough to push deletedRatio > 0.8.
    // deletedRatio = deleted / occupied, so we choose about 5/6.
    int toDelete = (count * 5) / 6 + 1;  // ensures > ~0.83
    int deleted = 0;
    bool started = false;

    for (i = 0; i < count && deleted < toDelete; i++) {
        if (inserted[i]) {
            Person victim(keys[i], ids[i], true);
            if (!c.remove(victim)) {
                return false;
            }
            deleted++;

            if (c.m_oldTable != NULL) {
                started = true;
            }
        }
    }

    // Test passes if a rehash was triggered at any point during or immediately after these deletions.
    return started;
}



/* Test: Rehash completion (deleted ratio) */
bool Tester::testRehashCompletionDeletedRatio() {
    // Start with a larger table so rehash can be attributed to deleted ratio, not load factor.
    Cache c(401, hashCode, LINEAR);
    Random keyRng(10000, 99999);
    Random idRng(MINID, MAXID);

    const int TARGET_INSERT = 80;
    string keys[TARGET_INSERT];
    int ids[TARGET_INSERT];
    bool inserted[TARGET_INSERT];
    bool removed[TARGET_INSERT];

    int i = 0;

    for (i = 0; i < TARGET_INSERT; i++) {
        inserted[i] = false;
        removed[i] = false;
    }

    int count = 0;
    int attempts = 0;

    // Insert until there is TARGET_INSERT successful inserts or give up after 3 * TARGET_INSERT attempts.
    while (count < TARGET_INSERT && attempts < TARGET_INSERT * 3) {
        string key = "DRC" + to_string(keyRng.getRandNum());
        int id = idRng.getRandNum();
        Person p(key, id, true);

        if (c.insert(p)) {
            keys[count] = key;
            ids[count] = id;
            inserted[count] = true;
            count++;
        }

        attempts++;
    }

    // Make there were a reasonable number of inserts.
    if (count < 50) {
        return false;
    }

    // Ensure no rehash has happened yet.
    if (c.m_oldTable != NULL) {
        return false;
    }

    // Delete enough to push deletedRatio > 0.8 and start rehashing.
    int toDelete = (count * 5) / 6 + 1;
    int deleted = 0;
    bool started = false;

    for (i = 0; i < count && deleted < toDelete; i++) {
        if (inserted[i]) {
            Person victim(keys[i], ids[i], true);
            if (!c.remove(victim)) {
                return false;
            }
            removed[i] = true;
            deleted++;

            if (c.m_oldTable != NULL) {
                started = true;
            }
        }
    }

    // If deleted-ratio rehash never started, this test fails.
    if (!started) {
        return false;
    }

    // Drive the incremental rehash to completion by performing additional inserts; each insert will move another 25% of the old table.
    int safety = 0;
    while (c.m_oldTable != NULL && safety < 500) {
        string extraKey = "DRX" + to_string(keyRng.getRandNum()) + "_" + to_string(safety);
        int extraID = idRng.getRandNum();
        c.insert(Person(extraKey, extraID, true));
        safety++;
    }

    if (c.m_oldTable != NULL) {
        return false;
    }

    for (i = 0; i < count; i++) {
        Person f = c.getPerson(keys[i], ids[i]);

        if (removed[i]) {
            if (f.getUsed()) {
                return false;
            }
        }
        else if (inserted[i]) {
            if (!f.getUsed()) {
                return false;
            }
        }
        // entries that were never inserted are ignored
    }

    return true;
}



/* Test: Deleted bucket reuse */
bool Tester::testInsertDeletedReuse() {
    Cache c(101, hashCode, LINEAR);

    int id1 = MINID;
    int id2 = MINID + 1;

    if (!c.insert(Person("ABC", id1, true))) {
        return false;
    }
    if (!c.remove(Person("ABC", id1, true))) {
        return false;
    }
    if (!c.insert(Person("ABC", id2, true))) {
        return false;
    }

    Person f = c.getPerson("ABC", id2);
    return f.getUsed() && f.getID() == id2;
}


/* Test: Update ID success */
bool Tester::testUpdateIDSuccess() {
    Cache c(101, hashCode, LINEAR);

    int originalID = MINID + 10;
    int newID = MINID + 20;

    Person p("UPD", originalID, true);

    if (!c.insert(p)) {
        return false;
    }

    // Update the ID to a different valid value.
    if (!c.updateID(p, newID)) {
        return false;
    }

    Person f = c.getPerson("UPD", newID);
    return f.getUsed() && f.getID() == newID;
}


/* Test: Update ID failure */
bool Tester::testUpdateIDFailure() {
    Cache c(101, hashCode, LINEAR);
    Person fake("NOPE", 2222, true);
    return !c.updateID(fake, 9999);
}

/* Test: Invalid ID insertion */
bool Tester::testInvalidIDInsertion() {
    Cache c(101, hashCode, LINEAR);

    Person low("X", MINID - 1, true);
    Person high("Y", MAXID + 1, true);

    if (c.insert(low)) {
        return false;
    }
    if (c.insert(high)) {
        return false;
    }

    return true;
}

/* Test: Policy change after rehash */
bool Tester::testPolicyChangeAfterRehash() {
    Cache c(101, hashCode, LINEAR);
    c.changeProbPolicy(DOUBLEHASH);

    // Insert a guaranteed set of unique (key, ID) pairs so that the load factor will exceed 0.5 and trigger a rehash.
    int i = 0;
    while (c.m_oldTable == NULL && i < 200) {
        string key = "P" + to_string(i);
        int id = MINID + i;
        Person p(key, id, true);

        c.insert(p);
        i++;
    }

    // Once the first rehash is started, startRehash() must have set m_currProbing to the new policy.
    return (c.m_currProbing == DOUBLEHASH);
}


/* Test: Quadratic wrap-around */
bool Tester::testQuadraticWrap() {
    // Size 11 will be promoted to MINPRIME (101) by the constructor.
    Cache c(11, hashCode, QUADRATIC);

    int id1 = MINID;
    int id2 = MINID + 1;
    int id3 = MINID + 2;

    Person a("AA", id1, true);
    Person b("AA", id2, true);
    Person ccc("AA", id3, true);

    if (!c.insert(a)) {
        return false;
    }
    if (!c.insert(b)) {
        return false;
    }
    if (!c.insert(ccc)) {
        return false;
    }

    if (!c.getPerson("AA", id1).getUsed()) {
        return false;
    }
    if (!c.getPerson("AA", id2).getUsed()) {
        return false;
    }
    if (!c.getPerson("AA", id3).getUsed()) {
        return false;
    }

    return true;
}


/* Test: Double-hash correctness */
bool Tester::testDoubleHashStepsCorrect() {
    Cache c(101, hashCode, DOUBLEHASH);

    int id1 = MINID + 100;
    int id2 = MINID + 101;

    Person a("ZZ", id1, true);
    Person b("ZZ", id2, true);

    if (!c.insert(a)) {
        return false;
    }
    if (!c.insert(b)) {
        return false;
    }

    if (!c.getPerson("ZZ", id1).getUsed()) {
        return false;
    }
    if (!c.getPerson("ZZ", id2).getUsed()) {
        return false;
    }

    return true;
}


/* Test: Find stops on null bucket */
bool Tester::testFindStopsOnNull() {
    Cache c(101, hashCode, LINEAR);

    c.insert(Person("STOP", 1234, true));

    Person missing = c.getPerson("NOT", 5555);

    return !missing.getUsed();
}

/* main(): Run all tests */
int main() {
    Tester t;

    cout << "Test Insert Non-Colliding: " << (t.testInsertNonColliding() ? "PASSED" : "FAILED") << endl;
    cout << "Test Insert With Collisions: " << (t.testInsertWithCollisions() ? "PASSED" : "FAILED") << endl;
    cout << "Test Find Error Case: " << (t.testFindErrorCase() ? "PASSED" : "FAILED") << endl;
    cout << "Test Find Non-Colliding: " << (t.testFindNonColliding() ? "PASSED" : "FAILED") << endl;
    cout << "Test Find Colliding: " << (t.testFindColliding() ? "PASSED" : "FAILED") << endl;
    cout << "Test Remove Non-Colliding: " << (t.testRemoveNonColliding() ? "PASSED" : "FAILED") << endl;
    cout << "Test Remove Colliding: " << (t.testRemoveColliding() ? "PASSED" : "FAILED") << endl;
    cout << "Test Rehash Trigger Load Factor: " << (t.testRehashTriggerLoadFactor() ? "PASSED" : "FAILED") << endl;
    cout << "Test Rehash Completion Load Factor: " << (t.testRehashCompletionLoadFactor() ? "PASSED" : "FAILED") << endl;
    cout << "Test Rehash Trigger Deleted Ratio: " << (t.testRehashTriggerDeletedRatio() ? "PASSED" : "FAILED") << endl;
    cout << "Test Rehash Completion Deleted Ratio: " << (t.testRehashCompletionDeletedRatio() ? "PASSED" : "FAILED") << endl;
    cout << "Test Insert Deleted Reuse: " << (t.testInsertDeletedReuse() ? "PASSED" : "FAILED") << endl;
    cout << "Test Update ID Success: " << (t.testUpdateIDSuccess() ? "PASSED" : "FAILED") << endl;
    cout << "Test Update ID Failure: " << (t.testUpdateIDFailure() ? "PASSED" : "FAILED") << endl;
    cout << "Test Invalid ID Insertion: " << (t.testInvalidIDInsertion() ? "PASSED" : "FAILED") << endl;
    cout << "Test Policy Change After Rehash: " << (t.testPolicyChangeAfterRehash() ? "PASSED" : "FAILED") << endl;
    cout << "Test Quadratic Wrap: " << (t.testQuadraticWrap() ? "PASSED" : "FAILED") << endl;
    cout << "Test DoubleHash Steps: " << (t.testDoubleHashStepsCorrect() ? "PASSED" : "FAILED") << endl;
    cout << "Test Find Stops On Null: " << (t.testFindStopsOnNull() ? "PASSED" : "FAILED") << endl;

    return 0;
}
