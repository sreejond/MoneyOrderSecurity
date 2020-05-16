#ifndef MONEYORDER_H
#define MONEYORDER_H

#include <string>
#include <vector>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>

typedef unsigned long long ull;

namespace boost
{
    namespace serialization
    {
        class access;
    }
}

class MoneyOrder
{
    public:
        // Serialization expects the object to have a default constructor
        MoneyOrder() {};
        MoneyOrder(ull amountOfMoney, std::string uniqueString, std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > identities);
        virtual ~MoneyOrder();
        ull getAmountOfMoney();
        std::string getUniqueString();
        std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > getIdentity();

    protected:

    private:
        ull mAmountOfMoney;
        std::string mUniqueString;
        std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > mIdentities;

        // Allow serialization to access non-public data members.
        friend class boost::serialization::access;

        template<typename Archive>
        void serialize(Archive& ar, const unsigned version)
        {
            ar & mAmountOfMoney & mUniqueString & mIdentities;  // Simply serialize the data members of Obj
        }
};

#endif // MONEYORDER_H
