#include "marketplace.hpp"

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

void marketplace::publish(account_name username, std::string content)
{
    require_auth(username);
    //transferを打ってuser登録しないと実行できない
    _users.get(username, "User doesnt exist");

    uint64_t check = 0;

    for(auto &item : _owners)
    {
        if(item.content == content)
        {
            check++;
            break;
        }
    }

    if(check == 0)
    {
        _owners.emplace(get_self(), [&](auto &modified_user) 
        {
            modified_user.content_id = _owners.available_primary_key();
            modified_user.content = content;
            modified_user.ownername = username;
        });
    }
}

void marketplace::sell(uint64_t content_id, account_name owner, uint64_t value)
{
    require_auth(owner);
    _users.get(owner, "User doesnt exist");
    uint64_t check = 0;
    auto owner_check = _owners.find(content_id);
    eosio_assert(owner_check->ownername == owner, "Cant Sell Your Contents!");

    for(auto &item : _stores)
    {
        if(item.content_id == content_id)
        {
            check++;
            break;
        }
    }

    eosio_assert(check == 0 , "Already Sell Your Content!");

    _stores.emplace(get_self(), [&](auto &modified_user)
    {
        modified_user.store_id = _stores.available_primary_key();
        modified_user.content_id = content_id;
        modified_user.content = owner_check->content;
        modified_user.value = value;
    });
}

void marketplace::purchase(uint64_t store_id, account_name username)
{
    require_auth(username);
    _users.get(username, "User doesnt exist");

    auto store = _stores.find(store_id);
    auto user = _users.find(username);
    auto cid = _owners.find(store->content_id);
    auto owner = _users.find(cid->ownername);

    eosio_assert(store->value < user->amount, "You have not money! Go Back!");
    eosio_assert(cid->ownername != username, "This is your Content! Go Back!");

    uint64_t sum = user->amount - store->value;
    
    _users.modify(user, _self, [&](auto &modified_user) 
    {
        modified_user.amount = modified_user.amount - sum;
    });

    _users.modify(owner, _self, [&](auto &modified_user) {
        modified_user.amount = modified_user.amount + sum;
    });

    _owners.modify(cid, _self, [&](auto &modified_user) {
        modified_user.ownername = username;
    });
}

void marketplace::cancell(uint64_t store_id, account_name username)
{
    require_auth(username);
    _users.get(username, "User doesnt exist");

    auto store = _stores.find(store_id);
    auto cid = _owners.find(store->content_id);
    eosio_assert(cid->ownername == username, "This is not your Conrent!");

    if (store != _stores.end())
    {
        _stores.erase(store);
    }
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