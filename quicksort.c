// https://en.wikipedia.org/wiki/Quicksort

void swap(unsigned long long *a, unsigned long long *b) {
    unsigned long long c = *a;
    *a = *b;
    *b = c;
}

long partition (unsigned long long arr[], long low, long high) {
    long mid = (low + high) / 2;
    if (arr[mid] < arr[low]) {
        swap(&arr[low], &arr[mid]);
    } else if (arr[high] < arr[low]) {
        swap(&arr[low], &arr[high]);
    } else if (arr[mid] < arr[high]) {
        swap(&arr[mid], &arr[high]);
    }
    unsigned long long pivot = arr[high];
    long i = (low - 1);
 
    for (long j = low; j <= high- 1; j++) {
        if (arr[j] <= pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quickSort(unsigned long long arr[], long low, long high) {
    if (low < high) {
        long partition_index = partition(arr, low, high);
        quickSort(arr, low, partition_index - 1);
        quickSort(arr, partition_index + 1, high);
    }
}
