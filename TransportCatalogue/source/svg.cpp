#include "svg.h"

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        // Делегируем вывод тега своим подклассам
        RenderObject(context);
        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    // ----------- Polyline ---------------
    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(std::move(point));
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool state = true;
        for (const Point& p : points_) {
            if (state) {
                out << p.x << ","sv << p.y;
                state = false;
                continue;
            }
            out << " "sv << p.x << ","sv << p.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ----------- Text -------------------
    Text& Text::SetPosition(Point pos) {
        pos_ = std::move(pos);
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = std::move(offset);
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        size_ = std::move(size);
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = std::move(font_family);
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = std::move(font_weight);
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = std::move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << size_ << "\" "sv;
        if (!font_family_.empty()) {
            out << "font-family=\""sv << font_family_ << "\" "sv;
        }
        if (!font_weight_.empty()) {
            out << "font-weight=\""sv << font_weight_ << "\""sv;
        }
        RenderAttrs(out);
        out << ">"sv << data_ << "<"sv;
        out << "/text>"sv;

    }

    // ----------- Document ----------------
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        RenderContext context{ out, 0, 2 };
        for (const auto& obj : objects_) {
            obj->Render(context);
        }
        out << "</svg>"sv;
        //out << std::endl;
    }

    Polyline CreateStar(Point center, double outer_rad, double inner_rad, int num_rays) {
        Polyline polyline;
        for (int i = 0; i <= num_rays; ++i) {
            double angle = 2 * M_PI * (i % num_rays) / num_rays;
            polyline.AddPoint({ center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle) });
            if (i == num_rays) {
                break;
            }
            angle += M_PI / num_rays;
            polyline.AddPoint({ center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle) });
        }
        return polyline;
    }
}  // namespace svg

namespace shapes {
    // ----------- Triangle ----------------
    Triangle::Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
        : p1_(p1), p2_(p2), p3_(p3) {}

    void Triangle::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
    }

    // ------------- Star -----------------
    Star::Star(svg::Point center, double outer_radius, double inner_radius, int num_rays) :
        center_(center), outer_radius_(outer_radius), inner_radius_(inner_radius), num_rays_(num_rays) {}

    void Star::Draw(svg::ObjectContainer& container) const {
        container.Add(svg::CreateStar(center_, outer_radius_, inner_radius_, num_rays_).SetFillColor("red").SetStrokeColor("black"));
    }
    // ----------- Snowman ----------------
    Snowman::Snowman(svg::Point head_cender, double head_radius) {
        circles_.emplace_back(svg::Circle().SetCenter({ head_cender.x * 1, head_cender.y + (head_radius * 5) }).SetRadius(head_radius * 2).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        circles_.emplace_back(svg::Circle().SetCenter({ head_cender.x * 1, head_cender.y + (head_radius * 2) }).SetRadius(head_radius * 1.5).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
        circles_.emplace_back(svg::Circle().SetCenter(head_cender).SetRadius(head_radius).SetFillColor("rgb(240,240,240)").SetStrokeColor("black"));
    }

    void Snowman::Draw(svg::ObjectContainer& container) const {
        for (svg::Circle circle : circles_) {
            container.Add(circle);
        }
    }
}