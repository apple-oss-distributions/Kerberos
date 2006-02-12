/*
 * This template class has basically the same functionality as auto_ptr, but with
 * one important difference: it doesn't use operator delete to delete its pointer.
 * Instead, it takes the function to be used to delete the pointer as a template
 * argument. This allows us to specify different ways to delete different types, 
 * even different ways to delete the same type. See examples below.
 *
 * $Header$
 */

#ifndef UAutoPtr_h_
#define UAutoPtr_h_

#include <stdlib.h>

/*
 * The first template argument is the type that the pointer points to. 
 * The second is a helper class used to delete that type. For example, if
 * you want a pointer to type foo, and you need to use FreeFoo to free it,
 * you would do this:
 
class FooAutoPtrDeleter {
	public:
		static void Delete (Foo*	inFoo) { FreeFoo (inFoo); }
};

typedef UAutoPtr <Foo, FooAutoPtrDeleter>		FooAutoPtr;

 *
 *
 * This allows us to have multiple variants of UAutoPtr for the same type, and different
 * delete functions. This is useful because some APIs (for example, Profile API) overload
 * void* as the generic opaque type
 */

template <class T, class U>
class UAutoPtr {
	public:
		struct UAutoPtrRef {
			T*			mPtr;
		};
		
		explicit UAutoPtr (
			T*							inPointer = 0):
			mPointer (inPointer) {
		}
		
		UAutoPtr (
			UAutoPtr&				inOriginal):
			mPointer (inOriginal.Release ()) {
		}
		
		UAutoPtr&			operator = (
			UAutoPtr&		inOriginal) {
			Reset (inOriginal.Release ());
			return *this;
		}
		
		~UAutoPtr () {
			if (mPointer != NULL) {
				U::Delete (mPointer);
			}
		}
		
		T&						operator* () const {
			return *Get ();
		}
		
		T*						operator -> () const {
			return Get ();
		}
		
		T*						Get () const {
			return mPointer;
		}
		
		T*						Release () {
			T*	result = Get ();
			mPointer = NULL;
			return result;
		}
		
		void					Reset (
			T*							inNewPointer) {
			if (inNewPointer != Get ()) {
				if (mPointer != NULL)
					U::Delete (mPointer);
				mPointer = inNewPointer;
			}
		}
		
		UAutoPtr (
			UAutoPtrRef			inOriginal):
			mPointer (inOriginal.mPtr) {}
		
		UAutoPtr&			operator = (
			UAutoPtrRef	inOriginal) {
			Reset (inOriginal.mPtr);
			return *this;
		}
		
		operator UAutoPtrRef () {
			UAutoPtrRef		ref;
			ref.mPtr = Release ();
			return ref;
		}
		
		
	private:
		
		T*	mPointer;
};

/* We have to include a void partial specialization, because it can't have operator * */

template <class U>
class UAutoPtr <void, U> {
	public:
		struct UAutoPtrRef {
			void*			mPtr;
		};
		
		explicit UAutoPtr (
			void*							inPointer = 0):
			mPointer (inPointer) {
		}
		
		UAutoPtr (
			UAutoPtr&				inOriginal):
			mPointer (inOriginal.Release ()) {
		}
		
		UAutoPtr&			operator = (
			UAutoPtr&		inOriginal) {
			Reset (inOriginal.Release ());
			return *this;
		}
		
		~UAutoPtr () {
			if (mPointer != NULL) {
				U::Delete (mPointer);
			}
		}
		
		void*						operator -> () const {
			return Get ();
		}
		
		void*						Get () const {
			return mPointer;
		}
		
		void*						Release () {
			void*	result = Get ();
			mPointer = NULL;
			return result;
		}
		
		void					Reset (
			void*							inNewPointer) {
			if (inNewPointer != Get ()) {
				if (mPointer != NULL)
					U::Delete (mPointer);
				mPointer = inNewPointer;
			}
		}
		
		UAutoPtr (
			UAutoPtrRef			inOriginal):
			mPointer (inOriginal.mPtr) {}
		
		UAutoPtr&			operator = (
			UAutoPtrRef	inOriginal) {
			Reset (inOriginal.mPtr);
			return *this;
		}
		
		operator UAutoPtrRef () {
			UAutoPtrRef		ref;
			ref.mPtr = Release ();
			return ref;
		}
		
		
	private:
		
		void*	mPointer;
};

#endif /* UAutoPtr_h_ */