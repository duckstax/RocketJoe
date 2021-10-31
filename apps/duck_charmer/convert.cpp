#include "convert.hpp"

#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include "nlohmann/json.hpp"

#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

// The bug related to the use of RTTI by the pybind11 library has been fixed: a
// declaration should be in each translation unit.
PYBIND11_DECLARE_HOLDER_TYPE(T, boost::intrusive_ptr<T>)

using components::storage::document_t;
using json = nlohmann::json;

inline json to_json(const py::handle& obj) {
    if (obj.ptr() == nullptr || obj.is_none()) {
        return nullptr;
    }
    if (py::isinstance<py::bool_>(obj)) {
        return obj.cast<bool>();
    }
    if (py::isinstance<py::int_>(obj)) {
        return obj.cast<long>();
    }
    if (py::isinstance<py::float_>(obj)) {
        return obj.cast<double>();
    }
    if (py::isinstance<py::bytes>(obj)) {
        py::module base64 = py::module::import("base64");
        return base64.attr("b64encode")(obj).attr("decode")("utf-8").cast<std::string>();
    }
    if (py::isinstance<py::str>(obj)) {
        return obj.cast<std::string>();
    }
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        auto out = json::array();
        for (const py::handle value : obj) {
            out.push_back(to_json(value));
        }
        return out;
    }
    if (py::isinstance<py::dict>(obj)) {
        auto out = json::object();
        for (const py::handle key : obj) {
            out[py::str(key).cast<std::string>()] = to_json(obj[key]);
        }
        return out;
    }
    throw std::runtime_error("to_json not implemented for this type of object: " + py::repr(obj).cast<std::string>());
}

inline ::storage::retained_const_t<::storage::impl::value_t> to__(const py::handle& obj) {
    if (py::isinstance<py::bool_>(obj)) {
        return ::storage::impl::new_value(obj.cast<bool>());
    }
    if (py::isinstance<py::int_>(obj)) {
        return ::storage::impl::new_value(obj.cast<long>());
    }
    if (py::isinstance<py::float_>(obj)) {
        return ::storage::impl::new_value(obj.cast<double>());
    }
    if (py::isinstance<py::bytes>(obj)) {
        py::module base64 = py::module::import("base64");
        return ::storage::impl::new_value(base64.attr("b64encode")(obj).attr("decode")("utf-8").cast<std::string>());
    }
    if (py::isinstance<py::str>(obj)) {
        return ::storage::impl::new_value(obj.cast<std::string>());
    }

    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        auto out = document_t::create_array();
        for (const py::handle value : obj) {
            out->append(to__(value));
        }
        return out->as_array();
    }
    if (py::isinstance<py::dict>(obj)) {
        document_t out;
        to_document(obj, out);
        return out.value();
    }

    throw std::runtime_error("to_json not implemented for this type of object: " + py::repr(obj).cast<std::string>());
}

void to_document_inner(std::string&& key, const py::handle& source, document_t& target) {
    if (source.ptr() == nullptr || source.is_none()) {
        target.add_null(std::move(key));
        return;
    }
    if (py::isinstance<py::bool_>(source)) {
        target.add_bool(std::move(key), source.cast<bool>());
        return;
    }
    if (py::isinstance<py::int_>(source)) {
        target.add_long(std::move(key), source.cast<long>());
        return;
    }
    if (py::isinstance<py::float_>(source)) {
        target.add_double(std::move(key), source.cast<double>());
        return;
    }
    if (py::isinstance<py::bytes>(source)) {
        py::module base64 = py::module::import("base64");
        target.add_string(std::move(key), base64.attr("b64encode")(source).attr("decode")("utf-8").cast<std::string>());
        return;
    }
    if (py::isinstance<py::str>(source)) {
        target.add_string(std::move(key), source.cast<std::string>());
        return;
    }

    if (py::isinstance<py::tuple>(source) || py::isinstance<py::list>(source)) {
        auto inner_doc = document_t::create_array();
        for (const py::handle value : source) {
            inner_doc->append(to__(value));
        }
        target.add_array(std::move(key), inner_doc);
        return;
    }
    if (py::isinstance<py::dict>(source)) {
        document_t inner_doc;
        to_document(source, inner_doc);
        target.add_dict(std::move(key), std::move(inner_doc));
        return;
    }

    throw std::runtime_error("to_document not implemented for this type of object: " + py::repr(source).cast<std::string>());
}

void to_document(const py::handle& source, document_t& target) {
    for (const py::handle key : source) {
        to_document_inner(py::str(key).cast<std::string>(), source[key], target);
    }
}

void update_document_inner(std::string&& key, const py::handle& obj, document_t& target) {
    if (obj.ptr() == nullptr || obj.is_none()) {
        target.add_null(key);
        return;
    }
    if (py::isinstance<py::bool_>(obj)) {
        target.add_bool(key, obj.cast<bool>());
        return;
    }
    if (py::isinstance<py::int_>(obj)) {
        target.add_long(key, obj.cast<long>());
        return;
    }
    if (py::isinstance<py::float_>(obj)) {
        target.add_double(key, obj.cast<double>());
        return;
    }
    if (py::isinstance<py::bytes>(obj)) {
        py::module base64 = py::module::import("base64");
        target.add_string(key, base64.attr("b64encode")(obj).attr("decode")("utf-8").cast<std::string>());
    }
    if (py::isinstance<py::str>(obj)) {
        target.add_string(key, obj.cast<std::string>());
        return;
    }
    /*
    if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj)) {
        auto out = nl::json::array();
        for (const py::handle value : obj) {
            out.push_back(to_json(value));
        }
        return out;
    }
    if (py::isinstance<py::dict>(obj)) {
        auto out = nl::json::object();
        for (const py::handle key : obj) {
            out[py::str(key).cast<std::string>()] = to_json(obj[key]);
        }
        return out;
    }
     */
    throw std::runtime_error("update_document_inner not implemented for this type of object: " + py::repr(obj).cast<std::string>());
}

auto from_document(const document_t &document) -> py::object {
    //todo
//    if (document.is_null()) {
//        return py::none();
//    } else if (document.is_boolean()) {
//        return py::bool_(document.as_bool());
//    } else if (document.is_integer()) {
//        return py::int_(document.as_int32());
//    } else if (document.is_float()) {
//        return py::float_(document.as_double());
//    } else if (document.is_string()) {
//        return py::str(document.as_string());
//    } else if (document.is_array()) {
//        py::list list;
//        for (auto it : document.as_array()) {
//            list.append(from_document(it));
//        }
//        return std::move(list);
//    } else if (document.is_object()) {
//        py::dict dict;
//        for (auto it = document.cbegin(); it != document.cend(); ++it) {
//            dict[py::str(it.key())] = from_document(it.document());
//        }
//        return std::move(dict);
//    }
    return py::none();
}

auto from_object(const std::string& key, document_t& target) -> py::object {
    auto value = target.get(key);
//    return from_document(value);
}

void update_document(const py::handle& source, document_t& target) {
    for (const py::handle key : source) {
        //std::cerr << py::str(key).cast<std::string>()  << std::endl;
        update_document_inner(py::str(key).cast<std::string>(), source[key], target);
    }

    ///throw std::runtime_error("update_document not implemented for this type of object: " + py::repr(source).cast<std::string>());
}
