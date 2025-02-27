class NavRecord {

    public:
        NavRecord();

    protected:
        void setX(double value);
        void setY(double value);
        void setDepth(double value);

    private:
        double m_x;
        double m_y;
        double m_depth;
};
