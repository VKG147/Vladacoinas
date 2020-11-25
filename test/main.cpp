//merkle.cpp
#include <bitcoin/bitcoin.hpp>

// Merkle Root Hash
bc::hash_digest create_merkle(bc::hash_list &merkle) {
    // Stop if hash list is empty or contains one element
    if (merkle.empty())
        return bc::null_hash;
    else if (merkle.size() == 1)
        return merkle[0];
    // While there is more than 1 hash in the list, keep looping...
    while (merkle.size() > 1) {
        // If number of hashes is odd, duplicate last hash in the list.
        if (merkle.size() % 2 != 0)
            merkle.push_back(merkle.back());
        // List size is now even.
        assert(merkle.size() % 2 == 0);
        // New hash list.
        bc::hash_list new_merkle;
        // Loop through hashes 2 at a time.
        for (auto it = merkle.begin(); it != merkle.end(); it += 2) {
            // Join both current hashes together (concatenate).
            bc::data_chunk
            concat_data(bc::hash_size * 2);
            auto concat = bc::serializer<
                    decltype(concat_data.begin())>(concat_data.begin());
            concat.write_hash(*it);
            concat.write_hash(*(it + 1));
            // Hash both of the hashes.
            bc::hash_digest new_root = bc::bitcoin_hash(concat_data);
            // Add this to the new list.
            new_merkle.push_back(new_root);
        }
        // This is the new list.
        merkle = new_merkle;
        // DEBUG output -------------------------------------
        std::cout << "Current merkle hash list:" << std::endl;
        for (const auto &hash: merkle)
            std::cout << " " << bc::encode_base16(hash) << std::endl;
        std::cout << std::endl;
        // --------------------------------------------------
    }
    // Finally we end up with a single item.
    return merkle[0];
}

int main() {
    // Transactions hashes from a block (#100 001) to reproduce the same merkle root
    bc::hash_list tx_hashes{{
        bc::hash_literal("bb28a1a5b3a02e7657a81c38355d56c6f05e80b9219432e3352ddcfc3cb6304c"),
        bc::hash_literal("fbde5d03b027d2b9ba4cf5d4fecab9a99864df2637b25ea4cbcb1796ff6550ca"),
        bc::hash_literal("8131ffb0a2c945ecaf9b9063e59558784f9c3a74741ce6ae2a18d0571dac15bb"),
        bc::hash_literal("d6c7cb254aa7a5fd446e8b48c307890a2d4e426da8ad2e1191cc1d8bbe0677d7"),
        bc::hash_literal("ce29e5407f5e4c9ad581c337a639f3041b24220d5aa60370d96a39335538810b"),
        bc::hash_literal("45a38677e1be28bd38b51bc1a1c0280055375cdf54472e04c590a989ead82515"),
        bc::hash_literal("c5abc61566dbb1c4bce5e1fda7b66bed22eb2130cea4b721690bc1488465abc9"),
        bc::hash_literal("a71f74ab78b564004fffedb2357fb4059ddfc629cb29ceeb449fafbf272104ca"),
        bc::hash_literal("fda204502a3345e08afd6af27377c052e77f1fefeaeb31bdd45f1e1237ca5470"),
        bc::hash_literal("d3cd1ee6655097146bdae1c177eb251de92aed9045a0959edc6b91d7d8c1f158"),
        bc::hash_literal("cb00f8a0573b18faa8c4f467b049f5d202bf1101d9ef2633bc611be70376a4b4"),
        bc::hash_literal("05d07bb2de2bda1115409f99bf6b626d23ecb6bed810d8be263352988e4548cb")
    }};
    const bc::hash_digest merkle_root = create_merkle(tx_hashes);
    std::cout << "Merkle Root Hash: " << bc::encode_base16(merkle_root) << std::endl;
    // std::cout << "Merkle Root Hash-2: " << bc::encode_hash(merkle_root) << std::endl;
    return 0;
}