#include "marketplace.hpp"

void marketplace::transfer(uint64_t sender, uint64_t receiver)
{
    require_auth(sender);
}

void marketplace::publish(account_name username, std::string content)
{
    require_auth(username);
}

void marketplace::sell(uint64_t content_id, account_name owner, uint64_t value)
{
    require_auth(owner);
}

void marketplace::purchase(uint64_t store_id, account_name username)
{
    require_auth(username);
}

void marketplace::cancell(uint64_t store_id)
{
    return;
}

#define EOSIO_ABI_EX(TYPE, MEMBERS)                                                    \
    extern "C"                                                                         \
    {                                                                                  \
        void apply(uint64_t receiver, uint64_t code, uint64_t action)                  \
        {                                                                              \
            auto self = receiver;                                                      \
            if (code == self || code == N(eosio.token))                                \
            {                                                                          \
                if (action == N(transfer))                                             \
                {                                                                      \
                    eosio_assert(code == N(eosio.token), "Must transfer EOS");         \
                }                                                                      \
                TYPE thiscontract(self);                                               \
                switch (action)                                                        \
                {                                                                      \
                    EOSIO_API(TYPE, MEMBERS)                                           \
                }                                                                      \
                /* does not allow destructor of thiscontract to run: eosio_exit(0); */ \
            }                                                                          \
        }                                                                              \
    }

EOSIO_ABI_EX(marketplace, (transfer)(publish)(sell)(purchase)(cancell))