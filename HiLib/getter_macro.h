#pragma once

#define DECLARE_LAZY_GETTER(type, name) \
protected: mutable std::optional<type> m_opt##name; \
public: virtual const type& Get##name() const;

#define DECLARE_LAZY_GETTER_ARG1(type, name, arg1type) \
protected: mutable std::optional<type> m_opt##name; \
public: virtual const type& Get##name(const arg1type&) const

#define DECLARE_LAZY_COMPTR_GETTER(type, name) \
protected: mutable CComPtr<type> m_p##name; \
public: virtual const CComPtr<type>& Get##name##Ptr() const;



#define LAZY_GETTER(type, val) \
protected: std::optional<type> m_opt##val; \
protected: virtual void Load##val(); \
public: virtual void Clear##val() { m_opt##val = std::nullopt; } \
public: virtual const type& Get##val() { if (!m_opt##val.has_value()) { Load##val(); } return m_opt##val.value(); }

#define LAZY_GETTER_NO_CLEAR_IMPL(type, val) \
protected: std::optional<type> m_opt##val; \
protected: virtual void Load##val(); \
public: virtual void Clear##val(); \
public: virtual const type& Get##val() { if (!m_opt##val.has_value()) { Load##val(); } return m_opt##val.value(); }

#define SHAREDPTR_GETTER(type, val) \
protected: std::shared_ptr<type> m_sp##val; \
public: virtual const std::shared_ptr<type>& Get##val##Ptr() { return m_sp##val; }


#define LAZY_CCOMPTR_GETTER(type, val) \
protected: CComPtr<type> m_p##val; \
protected: virtual void Load##val##Ptr(); \
public: virtual void Clear##val##Ptr() { m_p##val = nullptr; } \
public: virtual const CComPtr<type>& Get##val##Ptr() { if (!m_p##val) { Load##val##Ptr(); } return m_p##val; }

#define STATIC_LAZY_CCOMPTR_GETTER(type, val) \
protected: static CComPtr<type> m_p##val; \
protected: static void Load##val##Ptr(); \
public: static void Clear##val##Ptr() { m_p##val = nullptr; } \
public: static const CComPtr<type>& Get##val##Ptr() { if (!m_p##val) { Load##val##Ptr(); } return m_p##val; }
