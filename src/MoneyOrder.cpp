#include "MoneyOrder.h"

MoneyOrder::MoneyOrder(ull amountOfMoney, std::string uniqueString, std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > identities)
: mAmountOfMoney(amountOfMoney)
, mUniqueString(uniqueString)
, mIdentities(identities)
{
}

MoneyOrder::~MoneyOrder()
{
}

ull MoneyOrder::getAmountOfMoney()
{
    return mAmountOfMoney;
}

std::string MoneyOrder::getUniqueString()
{
    return mUniqueString;
}

std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > MoneyOrder::getIdentity()
{
    return mIdentities;
}

