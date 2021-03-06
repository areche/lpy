/*  This file is part of copy_on_write_ptr.
    copy_on_write_ptr is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    copy_on_write_ptr is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with copy_on_write_ptr.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef COW_PTR_H
#define COW_PTR_H

#include <memory>

// The cow_ptr class implements copy-on-write semantics on top of std::shared_ptr
template <typename T,
          typename OwnershipFlag>
class copy_on_write_ptr {
   public:
      // === BASIC CLASS LIFECYCLE ===

      // Construct a cow_ptr from a raw pointer, acquire ownership.
      copy_on_write_ptr(T * ptr) :
         m_payload{ptr},
         m_ownership{true}
      { }

      // TODO: As a performance optimization, allow direct construction from a shared_ptr

      // Move-construct from a copy_on_write_ptr, acquire ownership.
      copy_on_write_ptr(copy_on_write_ptr && cptr) :
         m_payload{cptr.m_payload},
         m_ownership{true}
      { }

      // Copy-construct from a copy_on_write_ptr, DO NOT acquire ownership.
      copy_on_write_ptr(const copy_on_write_ptr & cptr) :
         m_payload{cptr.m_payload},
         m_ownership{false}
      { }

      // All our data members can take care of themselves on their own.
      ~copy_on_write_ptr() = default;

      // Moving a copy_on_write_ptr transfers ownership of the underlying data : nothing special
      copy_on_write_ptr & operator=(copy_on_write_ptr && cptr) = default;

      // Copying a copy_on_write_ptr DOES NOT transfer ownership of the underlying content, so we
      // need to reset our ownership bit in this scenario.
      copy_on_write_ptr & operator=(const copy_on_write_ptr & cptr) {
         m_ownership.set_ownership(false);
         m_payload = cptr.m_payload;
         return *this;
      }

      const T& operator*() const noexcept {
         return m_payload.operator*();
      }

      const T* operator->() const noexcept {
          return m_payload.operator->();
      }

      T& operator*() noexcept {
          copy_if_not_owner();
         return m_payload.operator*();
      }

      T* operator->() noexcept {
          copy_if_not_owner();
          return m_payload.operator->();
      }

      // === DATA ACCESS ===

      // Reading from copy-on-write data does not require ownership.
      // CAUTION: Be careful with references to non-const CoW data, as writes may invalidate them.
      const T & read() const { return *m_payload; }

      // Writing to copy-on-write data requires ownership, which must be acquired as needed.
      void write(const T & value) {
         copy_if_not_owner();
         *m_payload = value;
      }

      void write(T && value) {
         copy_if_not_owner();
         *m_payload = value;
      }

      bool operator ==(const copy_on_write_ptr & cptr) const {
          return m_payload == cptr.m_payload;
      }

   private:
      std::shared_ptr<T> m_payload;
      OwnershipFlag m_ownership;

      // If we are not the owner of the payload object, make a private copy of it
      void copy_if_not_owner() {
         m_ownership.acquire_ownership_once([this](){
            m_payload = std::make_shared<T>(*m_payload);
         });
      }
};

#endif
