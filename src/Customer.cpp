#include "Customer.h"
#include "Bank.h"
#include "Merchant.h"
#include "MoneyOrder.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <openssl/sha.h>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <math.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define NUM_OF_MONEY_ORDERS 10
#define UNIQUE_STRING_LENGTH 20
#define BANK_PUBLIC_KEY_E 227
#define BANK_PUBLIC_KEY_N 2059

Customer::Customer(ull amountOfMoney, ull accountNumber)
: mAmountOfMoney(amountOfMoney)
, mAccountNumber(accountNumber)
{
    srand(time(0));
    k = bigint("373");//randomCoprime(BANK_PUBLIC_KEY_N);
    kInv = bigint("1921");
    //cout << "k: " << k << endl;
    createIdentities(NUM_OF_MONEY_ORDERS);
    prepareMoneyOrders(NUM_OF_MONEY_ORDERS);
}

Customer::~Customer()
{
}

void Customer::setBank(Bank* bank)
{
    mBank = bank;
}

void Customer::setMerchant(Merchant* merchant)
{
    mMerchant = merchant;
}

string Customer::toString(ull number)
{
    stringstream ss;
    ss << number;
    string s=ss.str();
    return s;
}

bigint Customer::stringToInt(string str)
{
    string num = "";
    for (int i = 0; i < str.size(); i++)
    {
        ull n = str[i] - '0';
        num += toString(n);
    }
    return num;
}

string Customer::charArrayToString(unsigned char* charArray)
{
    char output[41];
    output[40] = NULL;
    for (ull i = 0; i < 20; i++)
    {

        sprintf(output+i*2, "%02x", charArray[i]);
    }

    string str( output, output + sizeof output / sizeof output[0] );
    return str;
}

ull Customer::alphaNumericStringToInteger(string alphaNumericString)
{
    long integer = 0;
    for (ull i = 0; i < alphaNumericString.size(); i++)
    {
        integer += alphaNumericString[i];
    }

    return integer;
}

void Customer::createIdentities(ull numOfMoneyOrders)
{
    for (ull i = 0; i < numOfMoneyOrders; i++)
    {
        ull randomNumber = rand();
        ull secretNumber = mAccountNumber ^ randomNumber;

        ull r1 = rand();
        ull r2 = rand();
        string leftStr = toString(randomNumber) + toString(r1) + toString(r2);
        unsigned char left[leftStr.length()];
        copy(leftStr.begin(), leftStr.end(), left);
        unsigned char leftOut[SHA_DIGEST_LENGTH];
        SHA1(left, sizeof(left), leftOut);
        string leftHash = charArrayToString(leftOut);
        mLeftHash.insert(make_pair(leftHash, make_pair(make_pair(r1, r2), randomNumber)));

        ull r3 = rand();
        ull r4 = rand();
        string rightStr = toString(secretNumber) + toString(r3) + toString(r4);
        unsigned char right[rightStr.length()];
        copy(rightStr.begin(), rightStr.end(), right);
        unsigned char rightOut[SHA_DIGEST_LENGTH];
        SHA1(right, sizeof(right), rightOut);
        string rightHash = charArrayToString(rightOut);
        mRightHash.insert(make_pair(rightHash, make_pair(make_pair(r3, r4), secretNumber)));

        mIdentities.push_back(make_pair(make_pair(leftHash, r1), make_pair(rightHash, r3)));
        //cout << leftHash << " " << rightHash << endl;
    }
}

string Customer::createUniqueString(ull uniqueStringLenght)
{
    string uniqueString = "";
    static const string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (ull i = 0; i < uniqueStringLenght; i++)
    {
        uniqueString += alphanum[rand() % alphanum.size()];
    }
    //cout << "U: " << uniqueString << endl;
    return uniqueString;
}

void Customer::prepareMoneyOrders(ull numOfMoneyOrders)
{
    ull cnt = 0;
    for (ull i = 0; i < numOfMoneyOrders; i++)
    {
        string uniqueString = createUniqueString(UNIQUE_STRING_LENGTH);
        //cout << i << "U: " << uniqueString << endl;
        MoneyOrder* moneyOrder = new MoneyOrder(mAmountOfMoney, uniqueString, mIdentities);

        //serialize
        // Create an output archive
        std::ostringstream os;
        boost::archive::binary_oarchive ar(os);
        // Write data
        ar & moneyOrder;
        string buf = os.str();

        unsigned char bufC[buf.length()];
        copy(buf.begin(), buf.end(), bufC);
        unsigned char bufHash[SHA_DIGEST_LENGTH];
        SHA1(bufC, sizeof(bufC), bufHash);
        string bufHashStr = charArrayToString(bufHash);

        //bigint bufInteger = stringToInt(bufHashStr);
        ull h = mHashFunction(bufHashStr) % 1013;
        bigint bufInteger = bigint(toString(h));

        std::stringstream buffer;
        buffer << bufInteger;
        string bufferInt = buffer.str();
        mMoneyOrdersHash.insert(make_pair(bufferInt, buf));
        //cout << i << " buf: " << bufInteger << " " << bufferInt<< endl;

        bigint blindedInteger = blind(bufInteger);
        std::stringstream bufBlinded;
        bufBlinded << blindedInteger;
        string blindedInt = bufBlinded.str();
        mBlindHash.insert(make_pair(blindedInt, bufferInt));
        //cout << "blind:" << blindedInteger << endl;
        mBlindedMoneyOrders.push_back(blindedInteger);
    }
}

bigint Customer::blind(bigint m)
{
    bigint tBigInt = bigint("1");
    bigint nBigInt = bigint(toString(BANK_PUBLIC_KEY_N));
    for (ull i = 1; i <= BANK_PUBLIC_KEY_E; i++)
    {
        tBigInt *= k;
    }

    tBigInt *= m;

    //cout << t << endl;
    tBigInt = tBigInt % nBigInt;
    //cout << t << endl;
    return tBigInt;
}

vector<MoneyOrder*> Customer::unblind(vector<bigint> blindedMoneyOrders)
{
    vector<MoneyOrder*> moneyOrders;
    for (ull i = 0; i != blindedMoneyOrders.size(); i++)
    {
        std::stringstream bufBlinded;
        bufBlinded << blindedMoneyOrders[i];
        string blindedInt = bufBlinded.str();

        string bufInteger = mBlindHash[blindedInt];
        string buf = mMoneyOrdersHash[bufInteger];

        // Restore data
        MoneyOrder* moneyOrderRestore;
        // Create and input archive
        std::istringstream is(buf);
        boost::archive::binary_iarchive ar(is);
        // Load
        ar & moneyOrderRestore;
        moneyOrders.push_back(moneyOrderRestore);
    }

    return moneyOrders;
}

ull Customer::randomCoprime(ull n)
{
     while (1)
     {
         ull r = rand() % n;
         if (r == 0 || r == 1)    continue;
         if (gcd(r, n) == 1) return r;
     }
}

ull Customer::gcd(ull a, ull b)
{
    return b == 0 ? a : gcd(b, a % b);
}

void Customer::provideMoneyOrdersToBank()
{
    mBank->receiveMoneyOrders(mBlindedMoneyOrders);
}

vector<pair<pair<pair<ull, ull>, ull>, pair<pair<ull, ull>, ull> > > Customer::revealIdentity()
{
    vector<pair<pair<pair<ull, ull>, ull>, pair<pair<ull, ull>, ull> > > revealIdentity;
    for (ull i = 0; i != mIdentities.size(); i++)
    {
        string leftHash = mIdentities[i].first.first;
        string rightHash = mIdentities[i].second.first;
        pair<pair<ull, ull>, ull> leftIdentity = mLeftHash[leftHash];
        pair<pair<ull, ull>, ull> rightIdentity = mRightHash[rightHash];
        revealIdentity.push_back(make_pair(leftIdentity, rightIdentity));
    }
    return revealIdentity;
}

vector<int> Customer::revealHalfIdentity(string selector)
{
    vector<int> revealIdentity;
    for (int i = 0; i != selector.size(); i++)
    {
        if (selector[i] == '0')
        {
            string leftHash = mIdentities[i].first.first;
            int randomNumber = mLeftHash[leftHash].second;
            revealIdentity.push_back(randomNumber);
        }
        else
        {
            string rightHash = mIdentities[i].second.first;
            int secretNumber = mRightHash[rightHash].second;
            revealIdentity.push_back(secretNumber);
        }
    }

    return revealIdentity;
}

void Customer::getSignMoneyOrder(bigint t)
{
    bigint nBigInt = bigint(toString(BANK_PUBLIC_KEY_N));
    mSignUnblinedMoneyOrder = (t * kInv) % nBigInt;

    bigint tBigInt = bigint("1");
    for (ull i = 1; i <= BANK_PUBLIC_KEY_E; i++)
    {
        tBigInt *= mSignUnblinedMoneyOrder;
    }

    mMoneyOrder = tBigInt % nBigInt;
}

bool Customer::spendWithMercant()
{
    std::stringstream buffer;
    buffer << mMoneyOrder;
    string moneyOrderStr = buffer.str();
    string buf = mMoneyOrdersHash[moneyOrderStr];
    //cout << "m: " << mMoneyOrder << " b: " << buf << endl;

    // Restore data
    MoneyOrder* moneyOrderRestore;
    // Create and input archive
    std::istringstream is(buf);
    boost::archive::binary_iarchive ar(is);
    // Load
    ar & moneyOrderRestore;

    bool isValid = mMerchant->spendMoneyOrder(mSignUnblinedMoneyOrder, moneyOrderRestore);
    if (!isValid)   return false;
    return true;
}
