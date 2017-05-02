#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph.h"

SubjectId subjectIdFromTriple(Triple triple) {
  return (SubjectId)((triple & SUBJECT_MASK) >> (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH));
}

PredicateId predicateIdFromTriple(Triple triple) {
  return (PredicateId)((triple & PREDICATE_MASK) >> OBJECT_BIT_WIDTH);
}

ObjectId objectIdFromTriple(Triple triple) {
  return (ObjectId)(triple & OBJECT_MASK);
}

EntityPair toSOEntry(SubjectId subject, ObjectId object) {
  return ((EntityPair)subject << 32) | (EntityPair)object;
}

EntityPair toOSEntry(ObjectId object, SubjectId subject) {
  return ((EntityPair)object << 32) | (EntityPair)subject;
}

EntityPair tripleToSOEntry(Triple triple) {
  return toSOEntry(subjectIdFromTriple(triple), objectIdFromTriple(triple));
}

EntityPair tripleToOSEntry(Triple triple) {
  return toOSEntry(objectIdFromTriple(triple), subjectIdFromTriple(triple));
}

Triple toTriple(SubjectId subject, PredicateId predicate, ObjectId object) {
  return (((Triple)subject) << (PREDICATE_BIT_WIDTH + OBJECT_BIT_WIDTH))
        | (((Triple)predicate) << OBJECT_BIT_WIDTH)
        | (Triple)object;
}

Triple toTripleFromSOEntry(EntityPair soPair, PredicateId predicate) {
  return toTriple((SubjectId)(soPair >> 32), predicate, (ObjectId)(soPair & 0xFFFFFFFF));
}

Triple toTripleFromOSEntry(EntityPair osPair, PredicateId predicate) {
  return toTriple((SubjectId)(osPair & 0xFFFFFFFF), predicate, (ObjectId)(osPair >> 32));
}

PredicateEntry* createPredicateEntry(PredicateId predicate) {
  PredicateEntry *entry = malloc(sizeof(PredicateEntry));
  entry->predicate = predicate;
  entry->entryCount = 0;
  entry->currentEntriesLength = PREDICATE_ENTRY_INITIAL_ALLOCATION_LENGTH;
  entry->soEntries = malloc(sizeof(EntityPair) * entry->currentEntriesLength);
  entry->osEntries = malloc(sizeof(EntityPair) * entry->currentEntriesLength);
  return entry;
}

void freePredicateEntry(PredicateEntry *entry) {
  free(entry->soEntries);
  free(entry->osEntries);
  free(entry);
}

void growPredicateEntry(PredicateEntry *entry) {
  entry->currentEntriesLength *= 2;
  entry->soEntries = realloc(entry->soEntries, sizeof(EntityPair) * entry->currentEntriesLength);
  entry->osEntries = realloc(entry->osEntries, sizeof(EntityPair) * entry->currentEntriesLength);
}

void addToPredicateEntry(PredicateEntry *entry, SubjectId subject, ObjectId object) {
  if ((entry->entryCount + 1) >= entry->currentEntriesLength) {
    growPredicateEntry(entry);
  }
  entry->soEntries[entry->entryCount] = toSOEntry(subject, object);
  entry->osEntries[entry->entryCount] = toOSEntry(object, subject);
  entry->entryCount++;
}

PredicateEntryIterator* createPredicateEntryIterator(PredicateEntry *entry) {
  PredicateEntryIterator *iterator = malloc(sizeof(PredicateEntryIterator));
  iterator->entry = entry;
  iterator->position = 0;
  iterator->done = 0;
  return iterator;
}

void freePredicateEntryIterator(PredicateEntryIterator *iterator) {
  free(iterator);
}

Triple iterate(PredicateEntryIterator *iterator) {
  if (iterator->position >= iterator->entry->entryCount) {
    iterator->done = 1;
    return 0;
  }
  return toTripleFromSOEntry(iterator->entry->soEntries[iterator->position++], iterator->entry->predicate);
}

void initialize() {
}
