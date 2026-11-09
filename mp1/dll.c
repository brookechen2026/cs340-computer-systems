
#include "dll.h"
#include <string.h>
#include <stdlib.h>

void dllInitc(dllListc *self) {
    self->head = NULL;
    self->tail = NULL;
}

void dllInits(dllLists *self) {
    self->head = NULL;
    self->tail = NULL;
}

void dllClearc(dllListc *self) {
    while (self->head) {
        dllNodec *tmp = self->head;
        self->head = tmp->next;
        free(tmp);
    }
    self->tail = NULL;
}


void dllClears(dllLists *self) {
    while (self->head) {
        dllNodes *tmp = self->head;
        self->head = tmp->next;
        if (tmp->data) free(tmp->data);  
        free(tmp);
    }
    self->tail = NULL;
}


void dllDetachc(dllNodec *self) {
    if (self->prev) self->prev->next = self->next;
    if (self->next) self->next->prev = self->prev;
    self->prev = NULL;
    self->next = NULL;
}

void dllDetachs(dllNodes *self) {
    if (self->prev) self->prev->next = self->next;
    if (self->next) self->next->prev = self->prev;
    self->prev = NULL;
    self->next = NULL;
}

dllNodec *dllFindc(const dllListc *self, char value) {
    dllNodec *cursor = self->head;
    while (cursor) {
        if (cursor->data == value) return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

dllNodes *dllFinds(const dllLists *self, const char *value) {
    if (!value) return NULL;

    dllNodes *cursor = self->head;
    while (cursor) {
        if (cursor->data && strcmp(cursor->data, value) == 0) return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

char dllPopc(dllListc *self, char ifEmpty) {
    if (!self->head) return ifEmpty;
    dllNodec *tmp = self->head;
    char ans = tmp->data;

    self->head = tmp->next;
    if (self->head) self->head->prev = NULL;
    else self->tail = NULL;

    free(tmp);
    return ans;
}


char *dllPops(dllLists *self, char *ifEmpty) {
    if (!self->head) return ifEmpty;
    dllNodes *tmp = self->head;

    char *ans = tmp->data;  

    self->head = tmp->next;
    if (self->head) self->head->prev = NULL;
    else self->tail = NULL;
    //free(tmp->data);
    free(tmp); 
    return ans;
    
}


char dllShiftc(dllListc *self, char ifEmpty) {
    if (!self->tail) return ifEmpty;
    dllNodec *tmp = self->tail;
    char ans = tmp->data;

    self->tail = tmp->prev;
    if (self->tail) self->tail->next = NULL;
    else self->head = NULL;

    free(tmp);
    return ans;
}


char *dllShifts(dllLists *self, char *ifEmpty) {
    if (!self->tail) return ifEmpty;
    dllNodes *tmp = self->tail;

    char *ans = tmp->data;  

    self->tail = tmp->prev;
    if (self->tail) self->tail->next = NULL;
    else self->head = NULL;
    free(tmp->data);
    free(tmp);  

    return ans;
}

void dllPushc(dllListc *self, char value) {
    dllNodec *tmp = malloc(sizeof(dllNodec));
    if (!tmp) return;

    tmp->data = value;
    tmp->next = self->head;
    tmp->prev = NULL;

    if (self->head) self->head->prev = tmp;
    self->head = tmp;
    if (!self->tail) self->tail = tmp;
}



void dllPushs(dllLists *self, char *value) {
    if (!value) return;

    dllNodes *tmp = malloc(sizeof(dllNodes));  
    if (!tmp) return;

    tmp->data = value;  

    tmp->next = self->head;
    tmp->prev = NULL;

    if (self->head) self->head->prev = tmp;
    self->head = tmp;
    if (!self->tail) self->tail = tmp;
}



void dllEnqueues(dllLists *self, char *value) {
    if (!value) return;

    dllNodes *tmp = malloc(sizeof(dllNodes));
    if (!tmp) return;

    tmp->data = malloc(strlen(value) + 1);
    if (!tmp->data) {
        free(tmp);
        return;
    }

    int i = 0;
    while (value[i] != '\0') {
        tmp->data[i] = value[i];
        i++;
    }
    tmp->data[i] = '\0';

    tmp->next = NULL;
    tmp->prev = self->tail;

    if (self->tail) self->tail->next = tmp;
    self->tail = tmp;
    if (!self->head) self->head = tmp;
}

void dllEnqueuec(dllListc *self, char value) {
    dllNodec *tmp = malloc(sizeof(dllNodec));
    if (!tmp) return;

    tmp->data = value;
    tmp->next = NULL;
    tmp->prev = self->tail;

    if (self->tail) {
        self->tail->next = tmp;
    }
    self->tail = tmp;

    if (!self->head) {
        self->head = tmp;
    }
}

