/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 4.0.2
 *
 * This file is not intended to be easily readable and contains a number of
 * coding conventions designed to improve portability and efficiency. Do not make
 * changes to this file unless you know what you are doing--modify the SWIG
 * interface file instead.
 * ----------------------------------------------------------------------------- */

#ifndef SWIG_amplpy_cplexmp_swig_WRAP_H_
#define SWIG_amplpy_cplexmp_swig_WRAP_H_

#include <map>
#include <string>


class SwigDirector_GenericCallback : public ampls::GenericCallback, public Swig::Director {

public:
    SwigDirector_GenericCallback(PyObject *self);
    virtual int doAddCut(ampls::Constraint const &c, int type);
    virtual int doAddCutSwigPublic(ampls::Constraint const &c, int type) {
      return ampls::GenericCallback::doAddCut(c,type);
    }
    virtual bool canDo(ampls::CanDo::Functionality f);
    virtual int run();
    virtual ~SwigDirector_GenericCallback();
    virtual int setHeuristicSolution(int nvars, int const *indices, double const *values);
    virtual int getSolution(int len, double *sol);
    virtual double getObj();
    virtual int getWhere();
    virtual char const *getWhereString();
    virtual char const *getMessage();
    virtual ampls::Where::CBWhere getAMPLSWhere();
    virtual ampls::Variant getValue(ampls::Value::CBValue v);
    virtual std::vector< double, std::allocator< double > > getValueArray(ampls::Value::CBValue v);

/* Internal director utilities */
public:
    bool swig_get_inner(const char *swig_protected_method_name) const {
      std::map<std::string, bool>::const_iterator iv = swig_inner.find(swig_protected_method_name);
      return (iv != swig_inner.end() ? iv->second : false);
    }
    void swig_set_inner(const char *swig_protected_method_name, bool swig_val) const {
      swig_inner[swig_protected_method_name] = swig_val;
    }
private:
    mutable std::map<std::string, bool> swig_inner;

#if defined(SWIG_PYTHON_DIRECTOR_VTABLE)
/* VTable implementation */
    PyObject *swig_get_method(size_t method_index, const char *method_name) const {
      PyObject *method = vtable[method_index];
      if (!method) {
        swig::SwigVar_PyObject name = SWIG_Python_str_FromChar(method_name);
        method = PyObject_GetAttr(swig_get_self(), name);
        if (!method) {
          std::string msg = "Method in class GenericCallback doesn't exist, undefined ";
          msg += method_name;
          Swig::DirectorMethodException::raise(msg.c_str());
        }
        vtable[method_index] = method;
      }
      return method;
    }
private:
    mutable swig::SwigVar_PyObject vtable[10];
#endif

};


class SwigDirector_CPLEXCallback : public ampls::CPLEXCallback, public Swig::Director {

public:
    SwigDirector_CPLEXCallback(PyObject *self);
    virtual int doAddCut(ampls::Constraint const &c, int type);
    virtual int doAddCutSwigPublic(ampls::Constraint const &c, int type) {
      return ampls::CPLEXCallback::doAddCut(c,type);
    }
    virtual bool canDo(ampls::CanDo::Functionality f);
    virtual int run();
    virtual ~SwigDirector_CPLEXCallback();
    virtual int setHeuristicSolution(int nvars, int const *indices, double const *values);
    virtual int getSolution(int len, double *sol);
    virtual double getObj();
    virtual int getWhere();
    virtual char const *getWhereString();
    virtual char const *getMessage();
    virtual ampls::Where::CBWhere getAMPLSWhere();
    virtual ampls::Variant getValue(ampls::Value::CBValue v);
    virtual std::vector< double, std::allocator< double > > getValueArray(ampls::Value::CBValue v);

/* Internal director utilities */
public:
    bool swig_get_inner(const char *swig_protected_method_name) const {
      std::map<std::string, bool>::const_iterator iv = swig_inner.find(swig_protected_method_name);
      return (iv != swig_inner.end() ? iv->second : false);
    }
    void swig_set_inner(const char *swig_protected_method_name, bool swig_val) const {
      swig_inner[swig_protected_method_name] = swig_val;
    }
private:
    mutable std::map<std::string, bool> swig_inner;

#if defined(SWIG_PYTHON_DIRECTOR_VTABLE)
/* VTable implementation */
    PyObject *swig_get_method(size_t method_index, const char *method_name) const {
      PyObject *method = vtable[method_index];
      if (!method) {
        swig::SwigVar_PyObject name = SWIG_Python_str_FromChar(method_name);
        method = PyObject_GetAttr(swig_get_self(), name);
        if (!method) {
          std::string msg = "Method in class CPLEXCallback doesn't exist, undefined ";
          msg += method_name;
          Swig::DirectorMethodException::raise(msg.c_str());
        }
        vtable[method_index] = method;
      }
      return method;
    }
private:
    mutable swig::SwigVar_PyObject vtable[10];
#endif

};


#endif
