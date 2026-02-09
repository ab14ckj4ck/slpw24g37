#include <stdint.h>
#include <stdio.h>


void getManufacturerId(char* man_id) {
  uint32_t ebx, ecx, edx;

  asm volatile("cpuid" : "=b" (ebx), "=d" (edx), "=c" (ecx) : "a" (0) : "cc", "memory");

  ((uint32_t*) man_id)[0] = ebx;
  ((uint32_t*) man_id)[1] = edx;
  ((uint32_t*) man_id)[2] = ecx;
}

void getProcessorBrand(char* brand) {
  for (int i = 0; i < 4; i++) {
    uint32_t eax, ebx, ecx, edx;
    eax = 0x80000002 + i;
    asm volatile ( "cpuid" : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (eax) : "cc", "memory");
    ((uint32_t*) brand)[i * 4] = eax;
    ((uint32_t*) brand)[i * 4 + 1] = ebx;
    ((uint32_t*) brand)[i * 4 + 2] = ecx;
    ((uint32_t*) brand)[i * 4 + 3] = edx;
  }
}

uint32_t getFeature(uint32_t feature_bit, uint32_t reg) {
  uint32_t eax, ebx, ecx, edx;
  asm volatile("movl $1, %%eax\n\t" "cpuid\n\t" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : : "cc", "memory");

  return (reg & (1 << feature_bit)) ? 1 : 0;
}

int main() {
  /* TODO: Find the processor's manufacturer ID string,
   *       the processor brand string, and the
   *       availability bits of the given features from the wiki
   *       using the cpuid instruction and inline assembly.
   *       Do not use builtin functions.
   *       Print the values to stdout in the given formats.
   *       Feel free to modify this file wherever you want.
   */

  char man_id[13];
  man_id[12] = '\0';

  getManufacturerId(man_id);
  printf("Manufacturer id: %s\n", man_id);

  char brand[100];
  brand[99] = '\0';

  getProcessorBrand(brand);

  printf("Processor brand string: %s\n", brand);

  uint32_t ebx, ecx, edx;
  asm volatile ("cpuid" : "=b" (ebx), "=c" (ecx), "=d" (edx) : "a" (1) : "cc", "memory");

  uint32_t sse = getFeature(25, edx);
  uint32_t msr = getFeature(5, edx);
  uint32_t cmov = getFeature(15, edx);
  uint32_t avx = getFeature(28, ecx);
  uint32_t avx2 = getFeature(5, ebx);
  uint32_t vmx = getFeature(5, ecx);
  uint32_t fma = getFeature(12, ecx);
  uint32_t hybrid = getFeature(16, ecx);

  printf("sse: %u, msr: %u, cmov: %u, avx: %u, avx2: %u, vmx: %u, fma: %u, hybrid: %u\n",
         sse, msr, cmov, avx, avx2, vmx, fma, hybrid);
}
