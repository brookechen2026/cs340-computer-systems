namespace LinkedList {

#ifndef NULL
#define NULL ((void*)0)
#endif


template <typename T> class List; // type predeclaration

/// A doubly-linked list node
template <typename T> class Node {
  T data;     /// The value stored in this node
  Node* next; /// points closer to the tail of the list
  Node* prev; /// points closer to the head of the list
public:

  /**
   * Removes this node from its list
   */
  void detach() {
    if (prev) prev->next = next;
    if (next) next->prev = prev;
    prev = next = NULL;
  }

  friend List<T>;
};


/// A doubly-linked list
template <typename T> class List {
  Node<T>* head; /// The top of the stack or front of the queue
  Node<T>* tail; /// The bottom of the stack of back of the queue
public:

  /**
   * Creates an empty list.
   */
  List() { head = tail = NULL; }

  /**
   * Creates a singleton list (containing exactly 1 value)
   * 
   * @param value  the value to store in the new list
   */
  List(T& value) { head = tail = new Node<T>(); head->data = value; }

  /**
   * Destroys the list, and all nodes inside it (but not values stored in them)
   */
  ~List() { while(head) this->pop(T()); }

  /**
   * Find a value in the list.
   * 
   * @param value  the value to search for
   * @return a pointer to the node containing the item, or NULL if not found
   */
  Node<T>* find(const T value) const {
    Node<T>* cursor = head;
    while(cursor) {
      if (cursor->data == value) return cursor;
      cursor = cursor->next;
    }
    return NULL;
  }
  
  /**
   * Remove and return the value at the head of the list.
   * This is a stack pop operation and a queue remove operation.
   * 
   * @param ifEmpty  the value to return if the list is empty.
   * @return the value removed, or ifEmpty if there is not such value.
   */
  T pop(T ifEmpty) {
    if (!head) return ifEmpty;
    Node<T>* tmp = head;
    head = tmp->next;
    if (!tmp->next) head = tail = NULL;
    tmp->detach();
    T ans = tmp->data;
    delete tmp;
    return ans;
  }

  /**
   * Remove and return the value at the tail of the list.
   * This is not used by stacks or queues, but is included for completeness.
   * 
   * @param ifEmpty  the value to return if the list is empty.
   * @return the value removed, or ifEmpty if there is not such value.
   */
  T shift(T ifEmpty) {
    if (!tail) return ifEmpty;
    Node<T>* tmp = tail;
    if (!tmp->prev) head = tail = NULL;
    tail = tmp->prev;
    tmp->detach();
    T ans = tmp->data;
    delete tmp;
    return ans;
  }

  /**
   * Add a new value to the head of the list.
   * This is the stack push operation.
   * 
   * @param value  the value to add to the list.
   */
  void push(T value) {
    Node<T>* tmp = new Node<T>();
    tmp->data = value;
    tmp->next = head;
    tmp->prev = NULL;
    if (head) head->prev = tmp;
    head = tmp;
    if (!tail) tail = head;
  }

  /**
   * Add a new value to the tail of the list.
   * This is the queue insert operation.
   * 
   * @param value  the value to add to the list.
   */
  void enqueue(T value) {
    Node<T>* tmp = new Node<T>();
    tmp->data = value;
    tmp->prev = tail;
    tmp->next = NULL;
    if (tail) tail->next = tmp;
    tail = tmp;
    if (!head) head = tail;
  }
};



}
