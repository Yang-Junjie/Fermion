#pragma once
namespace Fermion
{
    enum class KeyCode : int
    {
        None = 0,

        LeftShift = 1,
        RightShift = 2,
        LeftControl = 3,
        RightControl = 4,
        LeftAlt = 5,
        RightAlt = 6,

        Space = 7,
        Enter = 8,
        Delete = 9,
        Escape = 10,

        Up = 11,
        Down = 12,
        Left = 13,
        Right = 14,

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90

    };
    enum class CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    };
}