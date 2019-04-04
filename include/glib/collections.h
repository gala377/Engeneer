//
// Created by rafalgal on 08.03.19.
//

#ifndef TKOM2_GLIB_COLLECTIONS_H
#define TKOM2_GLIB_COLLECTIONS_H

namespace glib::collections {

    template <typename T>
    class CollectionConstIter {
    public:
        explicit CollectionConstIter(const T& collection): collection(collection) {};

        typedef typename T::const_iterator const_iterator;

        const_iterator begin() {
            return collection.begin();
        }

        const_iterator end() {
            return collection.end();
        }

    private:
        const T& collection;
    };

}

#endif //TKOM2_COLLECTIONS_H
