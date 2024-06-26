#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        list_del(&entry->list);
        q_release_element(entry);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));


    if (!node) {
        return false;
    }

    if (!(node->value = strdup(s))) {
        free(node);
        return false;
    }

    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *node = malloc(sizeof(element_t));

    if (!node)
        return false;

    if (!(node->value = strdup(s))) {
        free(node);
        return false;
    }

    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *first_entry = list_first_entry(head, element_t, list);

    list_del(&first_entry->list);

    if (sp) {
        strncpy(sp, first_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return first_entry;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *end_entry = list_last_entry(head, element_t, list);

    list_del(&end_entry->list);

    if (sp) {
        strncpy(sp, end_entry->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }

    return end_entry;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || !head->next)
        return false;

    struct list_head *slow = head, *fast = head;

    while (fast->next != head && fast->next->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }

    element_t *mid = list_entry(slow->next, element_t, list);

    list_del(slow->next);
    q_release_element(mid);
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;

    bool isdup = false;
    element_t *node, *safe;

    list_for_each_entry_safe (node, safe, head, list) {
        if (node->list.next != head && strcmp(node->value, safe->value) == 0) {
            list_del(&node->list);
            q_release_element(node);
            isdup = true;
        } else if (isdup) {
            list_del(&node->list);
            q_release_element(node);
            isdup = false;
        }
    }
    return true;
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || !head->next || !head->next->next)
        return;

    q_reverseK(head, 2);
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;

    struct list_head *node, *safe;

    list_for_each_safe (node, safe, head) {
        list_move(node, head);
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || k == 1)
        return;

    int cnt = 0;
    struct list_head *node, *safe, *start = head, tmp_head;

    INIT_LIST_HEAD(&tmp_head);
    list_for_each_safe (node, safe, head) {
        cnt++;
        if (cnt == k) {
            cnt = 0;
            list_cut_position(&tmp_head, start, node);
            q_reverse(&tmp_head);
            list_splice_init(&tmp_head, start);
            start = safe->prev;
        }
    }
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

void merge_two_sorted_list(struct list_head *l1,
                           struct list_head *l2,
                           bool descend)
{
    struct list_head merge;
    INIT_LIST_HEAD(&merge);

    while (!list_empty(l1) && !list_empty(l2)) {
        element_t *n1 = list_entry(l1->next, element_t, list);
        element_t *n2 = list_entry(l2->next, element_t, list);

        if ((strcmp(n1->value, n2->value) > 0 && descend) ||
            (strcmp(n1->value, n2->value) < 0 && !descend))
            list_move_tail(&n1->list, &merge);
        else
            list_move_tail(&n2->list, &merge);
    }

    if (list_empty(l1))
        list_splice_tail(l2, &merge);
    else
        list_splice_tail(l1, &merge);

    INIT_LIST_HEAD(l1);
    INIT_LIST_HEAD(l2);

    list_splice(&merge, l1);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head, *fast = head, tmp_head;
    INIT_LIST_HEAD(&tmp_head);

    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_cut_position(&tmp_head, head, slow);

    q_sort(&tmp_head, descend);

    q_sort(head, descend);

    merge_two_sorted_list(head, &tmp_head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;
    int cnt = 1;
    struct list_head *slow = head->prev;
    struct list_head *fast = head->prev->prev;
    while (fast != head) {
        element_t *n1 = list_entry(slow, element_t, list);
        element_t *n2 = list_entry(fast, element_t, list);
        if (strcmp(n1->value, n2->value) < 0) {
            list_del(&n2->list);
            q_release_element(n2);
            fast = slow->prev;
        } else {
            cnt++;
            slow = slow->prev;
            fast = fast->prev;
        }
    }
    return cnt;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return 1;
    int cnt = 1;
    struct list_head *slow = head->prev;
    struct list_head *fast = head->prev->prev;
    while (fast != head) {
        element_t *n1 = list_entry(slow, element_t, list);
        element_t *n2 = list_entry(fast, element_t, list);
        if (strcmp(n1->value, n2->value) > 0) {
            list_del(&n2->list);
            q_release_element(n2);
            fast = slow->prev;
        } else {
            cnt++;
            slow = slow->prev;
            fast = fast->prev;
        }
    }
    return cnt;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return list_entry(head->next, queue_contex_t, chain)->size;

    queue_contex_t *first = list_entry(head->next, queue_contex_t, chain);
    queue_contex_t *entry, *safe;
    int sorted_size = first->size;


    list_for_each_entry_safe (entry, safe, head, chain) {
        if (!entry || entry == first)
            continue;
        sorted_size += entry->size;
        merge_two_sorted_list(first->q, entry->q, descend);
        entry->size = 0;
    }
    return sorted_size;
}
