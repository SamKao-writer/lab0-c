#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


static struct list_head *merge_two_lists(struct list_head *left,
                                         struct list_head *right);
static struct list_head *merge_sort(struct list_head *head);

/* Merge two lists. */
static struct list_head *merge_two_lists(struct list_head *left,
                                         struct list_head *right)
{
    struct list_head *head = NULL, **indirect = &head, **node;
    for (; left && right; *node = (*node)->next) {
        element_t *left_ele = list_entry(left, element_t, list);
        element_t *right_ele = list_entry(right, element_t, list);
        node =
            (strcmp(left_ele->value, right_ele->value) <= 0) ? &left : &right;
        *indirect = *node;
        indirect = &(*indirect)->next;
    }

    *indirect = (struct list_head *) ((unsigned long long) left |
                                      (unsigned long long) right);
    return head;
}

static struct list_head *merge_sort(struct list_head *head)
{
    if (!head->next)
        return head;

    /* Using fast and slow pointer to find the middle node of the list. */
    struct list_head *slow = head;
    for (struct list_head *fast = slow->next; fast && fast->next;
         fast = fast->next->next)
        slow = slow->next;

    /* Get the middle node and cut the list into left and right part. */
    struct list_head *middle = slow->next;
    slow->next = NULL;

    /* Recursive call */
    struct list_head *left = merge_sort(head);
    struct list_head *right = merge_sort(middle);

    /* merge left and right lists. */
    return merge_two_lists(left, right);
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head) * 1);
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *node;
    struct list_head *safe;
    list_for_each_safe (node, safe, head) {
        element_t *now = list_entry(node, element_t, list);
        list_del(node);
        q_release_element(now);
    }
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *new = malloc(sizeof(element_t) * 1);
    if (!new)
        return false;
    // INIT_LIST_HEAD(&new->list);
    new->value = test_strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    // if (!head)
    //     return false;
    // element_t *new = malloc(sizeof(element_t) * 1);
    // if (!new)
    //     return false;
    // // INIT_LIST_HEAD(&new->list);
    // new->value = test_strdup(s);
    // if (!new->value) {
    //     free(new);
    //     return false;
    // }
    // list_add_tail(&new->list, head);
    // return true;
    if (!head)
        return false;
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *removed = list_entry(head->next, element_t, list);
    if (sp && bufsize > 0) {
        strncpy(sp, removed->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(head->next);
    return removed;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    else if (list_is_singular(head))
        return q_remove_head(head, sp, bufsize);
    return q_remove_head(head->prev->prev, sp, bufsize);
    // if (!head || list_empty(head))
    //     return NULL;
    // // element_t *removed = list_entry(head->prev, element_t, list);
    // // if (sp && bufsize > 0) {
    // //     strncpy(sp, removed->value, bufsize - 1);
    // //     sp[bufsize - 1] = '\0';
    // // }
    // // list_del(head->prev);
    // // return removed;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int count = 0;
    struct list_head *node;
    list_for_each (node, head)
        count++;
    return count;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    struct list_head **indirect = &head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        indirect = &(*indirect)->next;
    }
    struct list_head *next = (*indirect)->next;
    next->prev = (*indirect)->prev;
    element_t *deleted = list_entry(*indirect, element_t, list);
    q_release_element(deleted);
    *indirect = next;
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    /* for base case */
    if (!head || list_empty(head))
        return false;

    /* 1. use list_for_each_entry_safe to test whether there are duplicate
     * nodes. */
    element_t *entry;
    element_t *safe;

    /* 2. use the flag to check out the status. */
    bool dup = false;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list != head && !strcmp(entry->value, safe->value)) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = true;
        } else if (dup) {
            list_del(&entry->list);
            q_release_element(entry);
            dup = false;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node;
    struct list_head *safe;
    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    /* for base case */
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    else if (k == 1)
        return;

    /* 1. first compute if there is k nodes in the list. */
    struct list_head *node;
    struct list_head *safe;
    struct list_head *curr = head;
    int count = 0;
    list_for_each_safe (node, safe, head) {
        if (count % k == k - 1) {
            /* 2. reverse the k-group */
            LIST_HEAD(head_to);
            list_cut_position(&head_to, curr, node);
            q_reverse(&head_to);
            list_splice_init(&head_to, curr);
            curr = safe->prev;
        }
        count++;
    }
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    /* Use recursive merge sort to complete sorting. */
    /* For base case */
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    /* Cut the next pointer to head of last node. */
    head->prev->next = NULL;
    head->next = merge_sort(head->next);

    /* Traverse the list and change the prev pointer of each node. */
    struct list_head *curr = head, *next = head->next;
    for (; curr && next; curr = next, next = next->next)
        next->prev = curr;

    curr->next = head;
    head->prev = curr;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    q_reverse(head);
    struct list_head *entry, *safe;
    char *string_min = NULL;
    list_for_each_safe (entry, safe, head) {
        element_t *ele = list_entry(entry, element_t, list);
        char *now_string = ele->value;
        if (!string_min)
            string_min = now_string;
        else if (strcmp(now_string, string_min) > 0) {
            list_del(entry);
            q_release_element(ele);
        } else if (strcmp(now_string, string_min) < 0) {
            string_min = now_string;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    q_reverse(head);
    struct list_head *entry, *safe;
    char *string_max = NULL;
    list_for_each_safe (entry, safe, head) {
        element_t *ele = list_entry(entry, element_t, list);
        char *now_string = ele->value;
        if (!string_max)
            string_max = now_string;
        else if (strcmp(now_string, string_max) < 0) {
            list_del(entry);
            q_release_element(ele);
        } else if (strcmp(now_string, string_max) > 0) {
            string_max = now_string;
        }
    }
    q_reverse(head);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    /* Traverse all the queues and cut the list and add to the first queue. */
    queue_contex_t *output = list_entry(head->next, queue_contex_t, chain);
    queue_contex_t *current = NULL;
    list_for_each_entry (current, head, chain) {
        if (current == output)
            continue;
        list_splice_init(current->q, output->q);
        output->size = output->size + current->size;
        current->size = 0;
    }
    q_sort(output->q, descend);
    return q_size(output->q);
}