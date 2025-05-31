#include <iostream>
#include <string>
#include <cstdio>
#include <unordered_map>
#include <algorithm>
#include <cstring>

typedef unsigned long long hash_t;
const int MAX_SEQ_LEN = 10005;
const unsigned long long BASE = 131;

// DNA sequence data
struct SequenceData {
    char ref[MAX_SEQ_LEN];
    char qry[MAX_SEQ_LEN];
    char rev_comp[MAX_SEQ_LEN];
    int ref_len, qry_len;
    hash_t hash_ref[MAX_SEQ_LEN];
    hash_t hash_qry[MAX_SEQ_LEN];
    hash_t hash_rev[MAX_SEQ_LEN];
    hash_t powers[MAX_SEQ_LEN];
} seq;

struct TraceData {
    int next_pos;
    int ref_pos;
    bool is_rev;
};

struct SegmentData {
    int start_pos;
    bool is_rev;
};

// Match results
struct MatchData {
    int end_idx;
    int len;
    bool is_rev;
    
    MatchData(int end, int len, bool rev)
        : end_idx(end), len(len), is_rev(rev) {}
    
    bool operator==(const MatchData& other) const {
        return end_idx == other.end_idx &&
               len == other.len &&
               is_rev == other.is_rev;
    }
};

// Hash function for MatchData
struct MatchHasher {
    std::size_t operator()(const MatchData& m) const {
        size_t val = m.end_idx;
        val = val * 10000 + m.len;
        val = val * 2 + m.is_rev;
        return val;
    }
};

// Global variables
int dp[MAX_SEQ_LEN];
TraceData trace[MAX_SEQ_LEN];
std::unordered_map<hash_t, SegmentData> seg_map;
std::unordered_map<MatchData, int, MatchHasher> freq_map;

// Forward declarations
void init_powers();
void read_files(const char* ref_path, const char* qry_path);
void calc_hashes();
void build_hash_map();
void solve_dp();
void find_patterns();
void show_results();

// DNA manipulation functions
void calc_hash(int start, int end, const char* sequence, hash_t* hash_array) {
    hash_t h = 0;
    for (int i = start; i < end; i++) {
        h = h * BASE + sequence[i];
        hash_array[i] = h;
    }
}

void generate_rev_comp(const char* original, char* result, int length) {
    // Reverse the sequence
    for (int i = 0; i < length; i++) {
        result[i] = original[length - i - 1];
    }
    result[length] = '\0';
    
    // Create complement
    for (int i = 0; i < length; i++) {
        switch (result[i]) {
            case 'A': result[i] = 'T'; break;
            case 'T': result[i] = 'A'; break;
            case 'C': result[i] = 'G'; break;
            case 'G': result[i] = 'C'; break;
        }
    }
}

hash_t get_substr_hash(int start, int end, const hash_t* hash_array) {
    if (start == 0) {
        return hash_array[end];
    }
    return hash_array[end] - hash_array[start - 1] * seq.powers[end - start + 1];
}

hash_t get_rev_hash(int start, int end, int total_length, const hash_t* hash_array) {
    return get_substr_hash(total_length - end - 1, total_length - start - 1, hash_array);
}

// Main functions
void init_powers() {
    seq.powers[0] = 1;
    for (int i = 1; i < MAX_SEQ_LEN; i++) {
        seq.powers[i] = seq.powers[i - 1] * BASE;
    }
}

void read_files(const char* ref_path, const char* qry_path) {
    FILE* qry_file = fopen(qry_path, "r");
    if (!qry_file) {
        std::cerr << "Error: Cannot open query file" << std::endl;
        exit(1);
    }
    
    FILE* ref_file = fopen(ref_path, "r");
    if (!ref_file) {
        std::cerr << "Error: Cannot open reference file" << std::endl;
        fclose(qry_file);
        exit(1);
    }
    
    fscanf(qry_file, "%s", seq.qry);
    fscanf(ref_file, "%s", seq.ref);
    
    seq.ref_len = strlen(seq.ref);
    seq.qry_len = strlen(seq.qry);
    
    fclose(qry_file);
    fclose(ref_file);
}

void calc_hashes() {
    // Calculate hashes for reference and query sequences
    calc_hash(0, seq.ref_len, seq.ref, seq.hash_ref);
    calc_hash(0, seq.qry_len, seq.qry, seq.hash_qry);
    
    // Create and hash the reverse complement of reference
    generate_rev_comp(seq.ref, seq.rev_comp, seq.ref_len);
    calc_hash(0, seq.ref_len, seq.rev_comp, seq.hash_rev);
}

void build_hash_map() {
    // Create hash map of all possible segments from reference sequence
    for (int start = 0; start < seq.ref_len; start++) {
        for (int end = start; end < seq.ref_len; end++) {
            // Calculate hash for normal segment
            hash_t fwd_hash = get_substr_hash(start, end, seq.hash_ref);
            if (seg_map.find(fwd_hash) == seg_map.end()) {
                seg_map[fwd_hash] = {start, false};
            }
            
            // Calculate hash for complement segment
            hash_t rev_hash = get_rev_hash(start, end, seq.ref_len, seq.hash_rev);
            if (seg_map.find(rev_hash) == seg_map.end()) {
                seg_map[rev_hash] = {start, true};
            }
        }
    }
}

void solve_dp() {
    // Initialize DP table with large values
    const int INF = 0x3f3f3f3f;
    std::fill(dp, dp + MAX_SEQ_LEN, INF);
    
    // Base case: end of sequence
    dp[seq.qry_len] = 0;
    
    // Dynamic programming to find optimal matching
    for (int start = seq.qry_len - 1; start >= 0; start--) {
        for (int end = start; end < seq.qry_len; end++) {
            hash_t seg_hash = get_substr_hash(start, end, seq.hash_qry);
            
            auto it = seg_map.find(seg_hash);
            if (it != seg_map.end()) {
                int next = end + 1;
                
                // Update if we found a better path
                if (dp[start] > 1 + dp[next]) {
                    dp[start] = 1 + dp[next];
                    trace[start] = {
                        next,
                        it->second.start_pos,
                        it->second.is_rev
                    };
                }
            }
        }
    }
}

void find_patterns() 
{
    
    // Trace the optimal path and identify patterns
    int pos = 0;
    while (pos < seq.qry_len) {
        int next = trace[pos].next_pos;
        
        // Safety check for invalid indices
        if (next <= pos || next > seq.qry_len) {
            std::cerr << "Error: Invalid position in trace" << std::endl;
            break;
        }
        
        // Create pattern match entry
        int seg_len = next - pos;
        int ref_end = trace[pos].ref_pos + seg_len - 1;
        MatchData match(ref_end, seg_len, trace[pos].is_rev);
        
        // Update pattern frequency
        freq_map[match]++;
        
        // Move to next position
        pos = next;
    }
}

void show_results() {
    
    std::cout << "| Location in ref |    Segment    | count | Size  | Reversed |\n";
    
    for (const auto& entry : freq_map) {
        const MatchData& match = entry.first;
        int count = entry.second;
        
        // Extract the matched sequence
        char pattern[16] = {0};
        int start_pos = match.end_idx - match.len + 1;
        
        if (match.len <= 10) {
            // For short patterns, show the entire sequence
            for (int i = 0; i < match.len; i++) {
                pattern[i] = seq.ref[start_pos + i];
            }
        } else {
            // For long patterns, show beginning and ellipsis
            for (int i = 0; i < 5; i++) {
                pattern[i] = seq.ref[start_pos + i];
            }
            strcpy(pattern + 5, "...");
        }
        
        printf("| %5d  -  %5d | %-13s | %5d | %5d | %-8s |\n", 
               start_pos, match.end_idx, pattern,
               count, match.len, match.is_rev ? "Yes" : "No");
    }
    
}


int main() {
    // Step 1: Initialize hash powers
    init_powers();
    
    // Step 2: Load sequence data
    read_files("ref.txt", "query.txt");
    
    // Step 3: Calculate hashes
    calc_hashes();
    
    // Step 4: Build hash map of all reference segments
    build_hash_map();
    
    // Step 5: Find optimal alignments using DP
    solve_dp();
    
    // Step 6: Identify repeat patterns
    find_patterns();
    
    // Step 7: Display results
    show_results();


    return 0;
}