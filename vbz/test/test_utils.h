#pragma once

#include <gsl/gsl>

#include <iostream>

namespace {

template <typename T, typename PrintType>
class ContainerDumper
{
public:
    ContainerDumper(T const& t)
    : m_data(t)
    {
    }

    T const& m_data;
};

template <typename PrintType, typename Container>
std::ostream& operator<<(std::ostream &str, ContainerDumper<Container, PrintType> const& container)
{
    str << "{ ";
    for (auto const& e : container.m_data)
    {
        str << PrintType(e) << ", ";
    }
    str << "}";
    return str;
}

template <typename PrintType, typename Container>
ContainerDumper<Container, PrintType> dump_explicit(Container const& c)
{
    return ContainerDumper<Container, PrintType>{ c };
}

template <typename Container>
ContainerDumper<Container, typename Container::value_type> dump(Container const& c)
{
    return ContainerDumper<Container, typename Container::value_type>{ c };
}

}