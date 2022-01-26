All this code was taken from https://github.com/moepinet/libmoepgf \
Since we only need gf256 for this FEC implementation (and also only need the "shuffle" implementations when optimized, since they are generally the fastest), 
I decided to just copy the right methods from the repository above in a header-only style.
NOTE: the optimized methods generally require a multiple of 8 or 16 bytes. The pattern when optimized is generally the same (mul,addmul):
1) use the fastest implementation for as many bytes as there are multiples of (8/16/32 depending on arch)
2) use the slow (table) implementation for the rest of the bytes
3) there is no optimized method available, flat table is used as a fallback

Also note: I only bothered to add NEON and SSSE3 shuffle optimized methods. In the rare case of NEON not being available on ARM,no 
optimization exists (one could try a left and right part table lookup without NEON) and the performance is therefore really bad. 
In the case of X86, AVX2 could provide a performance benefit compared to SSSE3, but I did not bother adding AVX2 support, since SSSE3 
is already fast enough for our use case.