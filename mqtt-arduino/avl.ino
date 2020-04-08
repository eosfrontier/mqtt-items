#ifdef MQTT_RFID

//#define AVL_DEBUG

#ifdef AVL_DEBUG
void avl_debug(char *fmt, ...)
{
  char dbgbuf[256];
  va_list args;
  va_start(args,fmt);
  vsnprintf(dbgbuf, sizeof(dbgbuf), fmt, args);
  Serial.print("avl debug:"); Serial.println(dbgbuf);
  va_end(args);
}
#else
#define avl_debug(...)
#endif

#define AVL_MAX_ENTRIES 1024

#define BITFIELD_GET(bitfield,offset,width)     (((bitfield) >> (offset)) & ((1 << (width)) - 1))
#define BITFIELD_SET(bitfield,offset,width,val) ((bitfield) = ((bitfield) & ~(((1 << (width))-1) << offset) | (((val) & ((1 << (width)) - 1)) << (offset))))

#define AVL_ACCESS_GET_BAL(node)       BITFIELD_GET((node)->bitfield,24,2)
#define AVL_ACCESS_GET_CHILD(node,idx) BITFIELD_GET((node)->bitfield,((idx)*12),12)

#define AVL_ACCESS_SET_BAL(node,val)       BITFIELD_SET((node)->bitfield,24,2,(val))
#define AVL_ACCESS_SET_CHILD(node,idx,val) BITFIELD_SET((node)->bitfield,((idx)*12),12,(val))

avl_access_t avl_access_tree[AVL_MAX_ENTRIES];

#define AVL_ENTRY(idx) (&avl_access_tree[(idx)-1])

int avl_root[2] = {0,0};

int avl_new_entry(avl_access_t *entry) {
    if ((avl_num_entries[0] + avl_num_entries[1]) >= AVL_MAX_ENTRIES) {
        avl_debug("AVL tree full, did not add entry with key %08x and data %08x", entry->key.v, entry->data.v);
        Serial.print("AVL tree full!  Cannot add key '"); Serial.print(entry->key.v); Serial.print("' with data '"); Serial.print(entry->data.v); Serial.println("'");
        return 0;
    }
    int ridx = (entry->bitfield ? 1 : 0);
    avl_num_entries[ridx]++;
    int node = avl_num_entries[0]+avl_num_entries[1];
    avl_access_t *newtreenode = AVL_ENTRY(node);
    newtreenode->key.v = entry->key.v;
    newtreenode->data.v = entry->data.v;
    newtreenode->bitfield = (2 << 24);
    avl_debug("Inserted node %d with key %08x and data %08x", node, newtreenode->key.v, newtreenode->data.v);
    return node;
}

int avl_rotate(int node, char l_r)
{
    avl_access_t *treenode = AVL_ENTRY(node);
    int newnode = AVL_ACCESS_GET_CHILD(treenode, l_r);
    avl_access_t *newtreenode = AVL_ENTRY(newnode);
    int newleaf = AVL_ACCESS_GET_CHILD(newtreenode, (1 - l_r));
    AVL_ACCESS_SET_CHILD(treenode, l_r, newleaf);
    AVL_ACCESS_SET_CHILD(newtreenode, (1 - l_r), node);
    return newnode;
}

int avl_insert_tree(int node, avl_access_t *entry)
{
    avl_access_t *treenode = AVL_ENTRY(node);
    avl_debug("Inserting new key %08x, looking at node %d with key %08x", entry->key.v, node, treenode->key.v);
    if (treenode->key.v == entry->key.v) {
        avl_debug("Found existing entry");
        treenode->data.v = entry->data.v;
        return 0;
    }
    char l_r = (treenode->key.v < entry->key.v) ? 1 : 0;
    int child = AVL_ACCESS_GET_CHILD(treenode, l_r);
    char heavy_l_r = AVL_ACCESS_GET_BAL(treenode);
    if (child) {
        int newchild = avl_insert_tree(child, entry); // Can return sentinel: 0 = end recursion, -1/-2 = tree height increase (child on left/right)
        if (newchild == 0) return 0; // Unwind recursion
        if (newchild > 0) {
            avl_debug("Tree modified, node %d new %d child is %d", node, l_r, newchild);
            AVL_ACCESS_SET_CHILD(treenode, l_r, newchild);
            return 0; // End recursion
        }
        int child_l_r = newchild + 2;
        if (heavy_l_r == 2) {
            avl_debug("Balanced, unbalancing to %d", l_r);
            // We're balanced, become unbalanced at child side
            AVL_ACCESS_SET_BAL(treenode, l_r);
            return l_r - 2; // Negative value as sentinel for height increase
        }
        if (l_r != heavy_l_r) {
            avl_debug("Unbalanced opposite, balancing to %d", l_r);
            // Height increase on opposite of heavy side.  Make balanced
            AVL_ACCESS_SET_BAL(treenode, 2); // 2 means balanced
            return 0; // End recursion
        }
        int newnode;
        avl_access_t *newtreenode;
        if (child_l_r != l_r) {
            avl_debug("Double rotate %d", l_r);
            avl_access_t *oldchild = AVL_ENTRY(child);
            // Double rotate
            int newchild = avl_rotate(child, child_l_r);
            AVL_ACCESS_SET_CHILD(treenode, l_r, newchild);
            newnode = avl_rotate(node, l_r);
            newtreenode = AVL_ENTRY(newnode);
            // oldchild = now on previous heavy side, newnode = new top, treenode = now on previous light side
            // Set new balances, the children of the new top are divided among the new children on either side (but swapped and flipped)
            char old_bal = AVL_ACCESS_GET_BAL(newtreenode);
            if (old_bal == l_r) {
                AVL_ACCESS_SET_BAL(treenode, 2);
                AVL_ACCESS_SET_BAL(oldchild, (1 - l_r));
            } else if (old_bal == (1 - l_r)) {
                AVL_ACCESS_SET_BAL(treenode, l_r);
                AVL_ACCESS_SET_BAL(oldchild, 2);
            } else {
                AVL_ACCESS_SET_BAL(treenode, 2);
                AVL_ACCESS_SET_BAL(oldchild, 2);
            }
        } else {
            avl_debug("Single rotate %d", l_r);
            // Single rotate
            newnode = avl_rotate(node, l_r);
            newtreenode = AVL_ENTRY(newnode);
            AVL_ACCESS_SET_BAL(treenode, 2);
        }
        AVL_ACCESS_SET_BAL(newtreenode, 2);
        return newnode;
    } else {
        int newnode = avl_new_entry(entry);
        if (!newnode) return 0;
        AVL_ACCESS_SET_CHILD(treenode, l_r, newnode);
        if (heavy_l_r != 2) {
            avl_debug("Inserting as second child");
            AVL_ACCESS_SET_BAL(treenode, 2);
            return 0;  // Sentinel: balanced node, end recursion
        } else {
            avl_debug("Inserting as first child");
            AVL_ACCESS_SET_BAL(treenode, l_r);
            return l_r-2; // Sentinel: Unbalanced node
        }
    }
}

// Insert a new characterid/cardid into the table
void avl_insert(avl_access_t *entry, int ridx)
{
    entry->bitfield = ridx;
    if (avl_root[ridx] == 0) {
        avl_debug("Inserting key %08x as root", entry->key.v);
        avl_root[ridx] = avl_new_entry(entry);
    } else {
        avl_debug("Inserting key %08x into tree", entry->key.v);
        int newroot = avl_insert_tree(avl_root[ridx], entry);
        if (newroot > 0) {
            avl_root[ridx] = newroot;
        }
    }
}

/*
// Update data.  NB: cardid is not set so this will have to use a table scan!
void avl_update_data(avl_access_t *entry)
{
    long character_id = entry->character_id;
    for (int i = 0; i < avl_num_entries; i++) {
        if (avl_access_tree[i].character_id == character_id) {
            AVL_ACCESS_SET_DATA(&avl_access_tree[i], entry->bitfield);
            return;
        }
    }
    Serial.print("Character ID "); Serial.print(character_id); Serial.print(" not found in list!");
}
*/

avl_access_t *avl_find(uint32_t key, int ridx)
{
    avl_debug("Looking for key %08x", key);
    int node = avl_root[ridx];
    while (node) {
        avl_access_t *treenode = AVL_ENTRY(node);
        avl_debug("Looking at node %d with key %08x", node, treenode->key.v);
        if (treenode->key.v == key) {
            avl_debug("found key %08x with data %08x", treenode->key.v, treenode->data.v);
            return treenode;
        }
        char l_r = (treenode->key.v < key) ? 1 : 0;
        node = AVL_ACCESS_GET_CHILD(treenode, l_r);
        avl_debug("Descending %s to node %d", (l_r ? "right" : "left"), node);
    }
    return NULL;
}

void avl_degrade_tree(int node)
{
  avl_access_t *treenode = AVL_ENTRY(node);
  int child = AVL_ACCESS_GET_CHILD(treenode, 0);
  if (child) avl_degrade_tree(child);
  if (treenode->data.access & 0x2) {
      treenode->data.access = treenode->data.access & ~0x2;
      avl_debug("Node %d access was fresh for key %08x, now set to %08x", node, treenode->key.v, treenode->data.v);
  } else {
      treenode->data.access = 0;
      avl_debug("Node %d access was stale for key %08x, now set to ", node, treenode->key.v, treenode->data.v);
  }
  child = AVL_ACCESS_GET_CHILD(treenode, 1);
  if (child) avl_degrade_tree(child);
}

void avl_degrade_access()
{
  avl_degrade_tree(avl_root[1]);
}

void avl_print_tree(int node, int depth)
{
#ifdef AVL_DEBUG
  if (node) {
    avl_access_t *treenode = AVL_ENTRY(node);
    avl_print_tree(AVL_ACCESS_GET_CHILD(treenode, 0), depth+1);
    avl_debug("Node %4d at depth %2d, key %08x, data %08x, balance %d, left %4d, right %4d", node, depth,
      treenode->key.v, treenode->data.v, AVL_ACCESS_GET_BAL(treenode), AVL_ACCESS_GET_CHILD(treenode, 0), AVL_ACCESS_GET_CHILD(treenode, 1));
    avl_print_tree(AVL_ACCESS_GET_CHILD(treenode, 1), depth+1);
  }
#endif
}

void avl_print_status()
{
    Serial.print("AVL tree 1 has "); Serial.print(avl_num_entries[0]); Serial.println(" entries");
    avl_print_tree(avl_root[0], 1);
    Serial.print("AVL tree 2 has "); Serial.print(avl_num_entries[1]); Serial.println(" entries");
    avl_print_tree(avl_root[1], 1);
}

#endif
