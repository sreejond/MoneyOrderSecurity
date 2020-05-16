#include "Merchant.h"
#include "Bank.h"
#include "Customer.h"
#include "MoneyOrder.h"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <openssl/sha.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define BANK_PUBLIC_KEY_E 227
#define BANK_PUBLIC_KEY_N 2059

Merchant::Merchant(Bank* bank, Customer* customer)
: mBank(bank)
, mCustomer(customer)
{
    srand(time(0));
}

Merchant::~Merchant()
{
}

string Merchant::toString(ull number)
{
    stringstream ss;
    ss << number;
    string s=ss.str();
    return s;
}

bigint Merchant::stringToInt(string str)
{
    string num = "";
    for (int i = 0; i < str.size(); i++)
    {
        ull n = str[i] - '0';
        num += toString(n);
    }
    return num;
}

string Merchant::charArrayToString(unsigned char* charArray)
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

bool Merchant::spendMoneyOrder(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrderOrig)
{
    mSignUnblinedMoneyOrder = signUnblinedMoneyOrder;
    mMoneyOrderOrig = moneyOrderOrig;

    bigint nBigInt = bigint(toString(BANK_PUBLIC_KEY_N));

    bigint tBigInt = bigint("1");
    for (ull i = 1; i <= BANK_PUBLIC_KEY_E; i++)
    {
        tBigInt *= signUnblinedMoneyOrder;
    }

    bigint moneyOrderGenerated = tBigInt % nBigInt;
    //cout << "message generated: " << moneyOrderGenerated << endl;

    /*****Hashing money order start *****/
    //serialize
    // Create an output archive
    std::ostringstream os;
    boost::archive::binary_oarchive ar(os);
    // Write data
    ar & moneyOrderOrig;
    string buf = os.str();

    unsigned char bufC[buf.length()];
    copy(buf.begin(), buf.end(), bufC);
    unsigned char bufHash[SHA_DIGEST_LENGTH];
    SHA1(bufC, sizeof(bufC), bufHash);
    string bufHashStr = charArrayToString(bufHash);
    //cout << "bufHash: " << bufHash << endl;

    //bigint bufInteger = stringToInt(bufHashStr);
    ull h = mHashFunction(bufHashStr) % 1013;
    bigint bufInteger = bigint(toString(h));
    /*****Hashing money order end *****/


    if (moneyOrderGenerated != bufInteger) return false;
    return true;
}

void Merchant::requestToRevealIdentity()
{
    string selector = "";
    for (int i = 0; i != mMoneyOrderOrig->getIdentity().size(); i++)
    {
        int r = rand() % 2;
        selector += r + '0';
    }

    mRevealIdentity = mCustomer->revealHalfIdentity(selector);
}

bool Merchant::placeMoneyOrder()
{
    bool isValid = mBank->takeMoneyOrder(mSignUnblinedMoneyOrder, mMoneyOrderOrig, mRevealIdentity);
    if (!isValid)   return false;
    return true;
}
