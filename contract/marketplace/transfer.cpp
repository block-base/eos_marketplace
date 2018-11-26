#include "marketplace.hpp"

// @abi action
void marketplace::transfer(uint64_t sender, uint64_t receiver)
{
    auto transfer_data = eosio::unpack_action_data<st_transfer>();
    eosio_assert(transfer_data.from != _self, "Cant send EOS Token!");

    auto s = _users.find(sender);

    if (s == _users.end())
    {
        _users.emplace(get_self(), [&](auto &modified_user) {
            modified_user.username = sender;
            modified_user.amount = (uint64_t)transfer_data.quantity.amount;
        });
        return;
    }

    _users.modify(s, get_self(), [&](auto &modified_user) {
        modified_user.amount = modified_user.amount + (uint64_t)transfer_data.quantity.amount;
    });
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

EOSIO_ABI_EX(marketplace, (transfer)