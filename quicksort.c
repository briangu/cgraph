// https://en.wikipedia.org/wiki/Quicksort

#include <stdio.h>

void swap(unsigned long long *a, unsigned long long *b) {
    unsigned long long c = *a;
    *a = *b;
    *b = c;
}

unsigned long partition(unsigned long long arr[], unsigned long low, unsigned long high) {
    unsigned long mid = (low + high) / 2;
    if (arr[mid] < arr[low]) {
        printf("1\n");
        swap(&arr[low], &arr[mid]);
    } else if (arr[high] < arr[low]) {
        printf("2\n");
        swap(&arr[low], &arr[high]);
    } else if (arr[mid] < arr[high]) {
        printf("3\n");
        swap(&arr[mid], &arr[high]);
    }
    unsigned long long pivot = arr[high];
    unsigned long i = (low - 1);
 
    for (unsigned long j = low; j <= high - 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(unsigned long long arr[], unsigned long low, unsigned long high) {
    if (low < high) {
        unsigned long partition_index = partition(arr, low, high);
        quicksort(arr, low, partition_index - 1);
        quicksort(arr, partition_index + 1, high);
    }
}
