#define _USE_MATH_DEFINES
#pragma once

#include <cmath>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

namespace svg {
    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    inline std::ostream& operator<<(std::ostream& out, std::optional<StrokeLineCap> type) {
        if (type == StrokeLineCap::BUTT) {
            return out << "butt";;
        }
        if (type == StrokeLineCap::ROUND) {
            return out << "round";
        }
        return out << "square";
    }

    inline std::ostream& operator<<(std::ostream& out, std::optional <StrokeLineJoin> type) {
        if (type == StrokeLineJoin::ARCS) {
            return out << "arcs";
        }
        if (type == StrokeLineJoin::BEVEL) {
            return out << "bevel";
        }
        if (type == StrokeLineJoin::MITER) {
            return out << "miter";
        }
        if (type == StrokeLineJoin::MITER_CLIP) {
            return out << "miter-clip";
        }
        return out << "round";
    }
   
    class Rgb {
    public:
        Rgb() = default;
        Rgb(uint8_t red_, uint8_t green_, uint8_t blue_) : red(red_), green(green_), blue(blue_) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    class Rgba : public Rgb {
    public:
        Rgba() = default;
        Rgba(uint8_t red_, uint8_t green_, uint8_t blue_, double opacity_) : Rgb(red_, green_, blue_), opacity(opacity_) {}
        double opacity = 1.0;
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    struct ColorOut {
        std::ostream& out;

        void operator()(std::monostate) const {
            out << "none";
        }

        void operator()(const std::string& color) const {
            out << color;
        }

        void operator()(const Rgb& rgb) {
            out << "rgb(" << unsigned(rgb.red) << "," << unsigned(rgb.green) << "," << unsigned(rgb.blue) << ")";

        }

        void operator()(const Rgba& rgba) {
            out << "rgba(" << unsigned(rgba.red) << "," << unsigned(rgba.green) << "," << unsigned(rgba.blue) << "," << rgba.opacity << ")";
        }
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }

        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    using Color = std::variant <std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{ "none" };

    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            width_ = std::move(width);
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {
            using namespace std::literals;
            if (fill_color_) {
                out << " fill=\""sv;
                std::visit(ColorOut{ out }, *fill_color_);
                out << "\""sv;

            }
            if (stroke_color_) {
                out << " stroke=\""sv;
                std::visit(ColorOut{ out }, *stroke_color_);
                out << "\""sv;
            }
            if (width_) {
                out << " stroke-width=\""sv << *width_ << "\""sv;
            }
            if (line_cap_) {
                out << " stroke-linecap=\""sv << line_cap_ << "\""sv;
            }
            if (line_join_) {
                out << " stroke-linejoin=\""sv << line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> width_;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;
    };

    /*
    * Абстрактный базовый класс Object служит для унифицированного хранения
    * конкретных тегов SVG-документа
    */
    class Object {
    public:

        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_ = { 0.0, 0.0 };
        double radius_ = 1.0;
    };

    class Polyline : public Object, public PathProps<Polyline> {
    public:
        // Добавляет очередную вершину к ломаной линии
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_ = {};
    };

    class Text : public Object, public PathProps<Text> {
    public:
        // Задаёт координаты опорной точки (атрибуты x и y)
        Text& SetPosition(Point pos);

        // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
        Text& SetOffset(Point offset);

        // Задаёт размеры шрифта (атрибут font-size)
        Text& SetFontSize(uint32_t size);

        // Задаёт название шрифта (атрибут font-family)
        Text& SetFontFamily(std::string font_family);

        // Задаёт толщину шрифта (атрибут font-weight)
        Text& SetFontWeight(std::string font_weight);

        // Задаёт текстовое содержимое объекта (отображается внутри тега text)
        Text& SetData(std::string data);

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;

        Point pos_ = { 0.0, 0.0 };
        Point offset_ = { 0.0, 0.0 };
        uint32_t size_ = 1;
        std::string font_family_ = {};
        std::string font_weight_ = {};
        std::string data_ = {};
    };

    class ObjectContainer {
    public:
        template <typename Obj>
        void Add(Obj obj);

        virtual ~ObjectContainer() = default;
    private:
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    };

    template <typename Obj>
    void ObjectContainer::Add(Obj obj) {
        AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& obj) const = 0;
        virtual ~Drawable() = default;
    };

    class Document : public ObjectContainer {
    public:
        // Добавляет в svg-документ объект-наследник svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;

        // Выводит в ostream svg-представление документа
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_ = {};
    };

    Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays);
}  // namespace svg

namespace shapes {

    class Triangle : public svg::Drawable {
    public:
        Triangle(svg::Point p1, svg::Point p2, svg::Point p3);

        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point p1_, p2_, p3_;
    };

    class Star : public svg::Drawable {
    public:
        Star(svg::Point center, double outer_radius, double inner_radius, int num_rays);

        void Draw(svg::ObjectContainer& container) const override;
    private:
        svg::Point center_;
        double outer_radius_, inner_radius_;
        int num_rays_;
    };

    class Snowman : public svg::Drawable {
    public:
        Snowman(svg::Point head_cender, double head_radius);

        void Draw(svg::ObjectContainer& container) const override;
    private:
        std::vector<svg::Circle> circles_;
    };
}// namespace shapes

