#ifdef MQTT_RFID

#define AVL_MAX_ENTRIES 1024

#define BITFIELD_GET(bitfield,offset,width)     (((bitfield) >> (offset)) & ((1 << (width)) - 1))
#define BITFIELD_SET(bitfield,offset,width,val) ((bitfield) = ((bitfield) & ~(((1 << (width))-1) << offset) | (((val) & ((1 << (width)) - 1)) << (offset))))

#define AVL_ACCESS_GET_DATA(node)      BITFIELD_GET((node)->bitfield,0,6)
#define AVL_ACCESS_GET_BAL(node)       BITFIELD_GET((node)->bitfield,6,2)
#define AVL_ACCESS_GET_CHILD(node,idx) BITFIELD_GET((node)->bitfield,(8+((idx)*12)),12)

#define AVL_ACCESS_SET_DATA(node,val)      BITFIELD_SET((node)->bitfield,0,6,(val))
#define AVL_ACCESS_SET_BAL(node,val)       BITFIELD_SET((node)->bitfield,6,2,(val))
#define AVL_ACCESS_SET_CHILD(node,idx,val) BITFIELD_SET((node)->bitfield,(8+((idx)*12)),12,(val))

avl_access_t avl_access_tree[AVL_MAX_ENTRIES];

#define AVL_ENTRY(idx) (&avl_access_tree[(idx)-1])

int avl_num_entries = 0;
int avl_root = 0;

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
    if (treenode->card_id == entry->card_id) {
        return 0;
    }
    char l_r = (treenode->card_id < entry->card_id) ? 1 : 0;
    int child = AVL_ACCESS_GET_CHILD(treenode, l_r);
    char heavy_l_r = AVL_ACCESS_GET_BAL(treenode);
    if (child) {
        int newchild = avl_insert_tree(child, entry); // Can return sentinel: 0 = end recursion, -1/-2 = tree height increase (child on left/right)
        if (newchild == 0) return 0; // Unwind recursion
        if (newchild > 0) {
            AVL_ACCESS_SET_CHILD(treenode, l_r, newchild);
            return 0; // End recursion
        }
        int child_l_r = newchild + 2;
        if (heavy_l_r == 2) {
            // We're balanced, become unbalanced at child side
            AVL_ACCESS_SET_BAL(treenode, l_r);
            return l_r - 2; // Negative value as sentinel for height increase
        }
        if (child_l_r != heavy_l_r) {
            // Height increase on opposite of heavy side.  Make balanced
            AVL_ACCESS_SET_BAL(treenode, 2); // 2 means balanced
            return 0; // End recursion
        }
        int newnode;
        avl_access_t *newtreenode;
        if (child_l_r != l_r) {
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
                AVL_ACCESS_SET_BAL(treenode, (1 - l_r));
                AVL_ACCESS_SET_BAL(oldchild, 2);
            } else if (old_bal == (1 - l_r)) {
                AVL_ACCESS_SET_BAL(treenode, 2);
                AVL_ACCESS_SET_BAL(oldchild, l_r);
            } else {
                AVL_ACCESS_SET_BAL(treenode, 2);
                AVL_ACCESS_SET_BAL(oldchild, 2);
            }
        } else {
            // Single rotate
            newnode = avl_rotate(node, l_r);
            newtreenode = AVL_ENTRY(newnode);
            AVL_ACCESS_SET_BAL(treenode, 2);
        }
        AVL_ACCESS_SET_BAL(newtreenode, 2);
        return newnode;
    } else {
        // No child, insert new leaf
        if (avl_num_entries >= AVL_MAX_ENTRIES) {
            Serial.print("AVL tree full!  Cannot add character '"); Serial.print(entry->character_id); Serial.print("' with card '"); Serial.print(entry->card_id); Serial.println("'");
            return 0;
        }

        avl_num_entries++;
        avl_access_t *newtreenode = AVL_ENTRY(avl_num_entries);
        newtreenode->card_id = entry->card_id;
        newtreenode->character_id = entry->character_id;
        newtreenode->bitfield = (entry->bitfield & 0x3f) | (2 << 6);
        // AVL_ACCESS_SET_DATA(newtreenode, entry->bitfield);
        // AVL_ACCESS_SET_AVL(newtreenode, 2);

        AVL_ACCESS_SET_CHILD(treenode, l_r, avl_num_entries);
        if (heavy_l_r != 2) {
            AVL_ACCESS_SET_BAL(treenode, 2);
            return 0;  // Sentinel: balanced node, end recursion
        } else {
            AVL_ACCESS_SET_BAL(treenode, l_r);
            return l_r-2; // Sentinel: Unbalanced node
        }
    }
}

// Insert a new characterid/cardid into the table
void avl_insert(avl_access_t *entry)
{
    if (avl_num_entries == 0) {
        avl_num_entries++;
        avl_access_t *newtreenode = AVL_ENTRY(avl_num_entries);
        newtreenode->card_id = entry->card_id;
        newtreenode->character_id = entry->character_id;
        newtreenode->bitfield = (entry->bitfield & 0x3f) | (2 << 6);
        avl_root = avl_num_entries;
    } else {
        int newroot = avl_insert_tree(avl_root, entry);
        if (newroot > 0) {
            avl_root = newroot;
        }
    }
}

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

avl_access_t *avl_find(uint32_t card_id)
{
    int node = avl_root;
    while (node) {
        avl_access_t *treenode = AVL_ENTRY(node);
        if (treenode->card_id == card_id) {
            return treenode;
        }
        char l_r = (treenode->card_id < card_id) ? 1 : 0;
        node = AVL_ACCESS_GET_CHILD(treenode, l_r);
    }
    return NULL;
}

void avl_print_status()
{
    Serial.print("AVL tree has "); Serial.print(avl_num_entries); Serial.println(" entries");
}

#endif
