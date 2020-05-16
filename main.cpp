#include <iostream>
#include "Customer.h"
#include "Merchant.h"
#include "Bank.h"

using namespace std;

int main()
{
    ull amountOfMoney, accountNumber;
    cout << "INPUT AMOUNT OF MONEY: ";
    cin >> amountOfMoney;
    cout << "INPUT ACCOUNT NUMBER: ";
    cin >> accountNumber;
    cout << endl;

    Customer* customer = new Customer(amountOfMoney, accountNumber);
    Bank* bank = new Bank(customer);
    Merchant* merchant = new Merchant(bank, customer);
    customer->setBank(bank);
    customer->setMerchant(merchant);
    bank->setMerchant(merchant);

    customer->provideMoneyOrdersToBank();
    bank->requestUnbind();
    bank->requestToRevealIdentity();
    bool isValid = bank->verifyMoneyOrders();
    if (isValid)    cout << "BANK: MONEY ORDERS ARE VALID\n";
    else cout << "BANK: MONEY ORDERS ARE NOT VALID\n";
    if (isValid)
    {
        bank->provideSignMoneyOrder();
        isValid = customer->spendWithMercant();
        if (isValid)    cout << "MERCHANT: ACCEPT MONEY ORDER\n";
        else cout << "MERCHANT: REJECT MONEY ORDER\n";
        if (isValid)
        {
            merchant->requestToRevealIdentity();
            isValid = merchant->placeMoneyOrder();
            if (isValid)    cout << "BANK: ACCEPT MONEY ORDER\n";
            else cout << "BANK: REJECT MONEY ORDER\n";
        }
    }
    return 0;
}
